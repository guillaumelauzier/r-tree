#include <vector>
#include <algorithm>

struct Rect {
    double minX, minY, maxX, maxY;
    Rect(double minX_, double minY_, double maxX_, double maxY_) : minX(minX_), minY(minY_), maxX(maxX_), maxY(maxY_) {}
};

struct RTreeNode {
    Rect rect;
    std::vector<RTreeNode*> children;
    RTreeNode(Rect rect_) : rect(rect_) {}
    ~RTreeNode() {
        for (RTreeNode* child : children) {
            delete child;
        }
    }
};

class RTree {
public:
    RTree(int maxChildren_) : maxChildren(maxChildren_) {}

    void insert(Rect rect) {
        if (!root) {
            root = new RTreeNode(rect);
        } else {
            insertRect(root, rect);
        }
    }

    std::vector<Rect> search(Rect rect) {
        std::vector<Rect> results;
        if (root) {
            searchRect(root, rect, results);
        }
        return results;
    }

private:
    RTreeNode* root = nullptr;
    int maxChildren;

    void insertRect(RTreeNode* node, Rect rect) {
        if (node->children.empty()) {
            // node is a leaf, add rectangle to it
            node->children.push_back(new RTreeNode(rect));
        } else {
            // node is an internal node, choose a child to insert rectangle
            int bestIndex = 0;
            double bestEnlargement = std::numeric_limits<double>::max();
            for (int i = 0; i < node->children.size(); i++) {
                double enlargement = calcEnlargement(node->children[i]->rect, rect);
                if (enlargement < bestEnlargement) {
                    bestIndex = i;
                    bestEnlargement = enlargement;
                }
            }
            insertRect(node->children[bestIndex], rect);
            // adjust bounds of the parent node
            node->rect = mergeRects(node->rect, rect);
        }
        if (node->children.size() > maxChildren) {
            // split node if it has exceeded maximum capacity
            RTreeNode* newNode = new RTreeNode(Rect(0, 0, 0, 0));
            splitNode(node, newNode);
            if (node == root) {
                root = new RTreeNode(Rect(0, 0, 0, 0));
                root->children.push_back(node);
            }
            root->children.push_back(newNode);
            adjustRect(root, newNode);
        }
    }

    void splitNode(RTreeNode* node, RTreeNode* newNode) {
        // choose two seeds as the rectangles that are farthest apart
        int seed1, seed2;
        findSeeds(node, seed1, seed2);
        newNode->rect = node->children[seed2]->rect;
        node->rect = node->children[seed1]->rect;
        // assign each remaining rectangle to the seed with which it has the greater overlap
        while (!node->children.empty()) {
            if (node->children.size() + newNode->children.size() == maxChildren + 1) {
                // assign remaining rectangles to the node with fewer children
                for (RTreeNode* child : node->children) {
                    newNode->children.push_back(child);
                }
                node->children.clear();
                break;
            }
            if (node->children.size() > newNode->children.size()) {
                int index;
                findBestChild(node, newNode->rect, index);
                newNode->children.push_back(node->children[index]);
                node->children.erase(node->children.begin() + index);
            } else {
                int index;
                findBestChild(newNode, node->rect, index);
                node->children.push_back(newNode->children[index]);
                newNode->children.erase(newNode->children.begin() + index);
            }
        }
    }

    void findSeeds(RTreeNode* node, int& seed1, int& seed2) {
        double maxDistance = 0.0;
        for (int i = 0; i < node->children.size(); i++) {
            for (int j = i + 1; j < node->children.size(); j++) {
                double distance = calcDistance(node->children[i]->rect, node->children[j]->rect);
                if (distance > maxDistance) {
                    seed1 = i;
                    seed2 = j;
                    maxDistance = distance;
                }
            }
        }
    }

    void findBestChild(RTreeNode* node, Rect rect, int& index) {
        double bestEnlargement = std::numeric_limits<double>::max();
        for (int i = 0; i < node->children.size(); i++) {
            double enlargement = calcEnlargement(node->children[i]->rect, rect);
            if (enlargement < bestEnlargement) {
                index = i;
                bestEnlargement = enlargement;
            }
        }
    }

    double calcEnlargement(Rect rect1, Rect rect2) {
        double maxX = std::max(rect1.maxX, rect2.maxX);
        double minX = std::min(rect1.minX, rect2.minX);
        double maxY = std::max(rect1.maxY, rect2.maxY);
        double minY = std::min(rect1.minY, rect2.minY);
        return (maxX - minX) * (maxY - minY) - rect1.maxX - rect1.minX - rect1.maxY - rect1.minY - rect2.maxX - rect2.minX - rect2.maxY - rect2.minY;
    }

    double calcDistance(Rect rect1, Rect rect2) {
        double dx = std::max(0.0, std::max(rect1.minX - rect2.maxX, rect2.minX - rect1.maxX));
        double dy = std::max(0.0, std::max(rect1.minY - rect2.maxY, rect2.minY - rect1.maxY));
        return dx * dx + dy * dy;
    }

    Rect mergeRects(Rect rect1, Rect rect2) {
        double minX = std::min(rect1.minX, rect2.minX);
        double minY = std::min(rect1.minY, rect2.minY);
        double maxX = std::max(rect1.maxX, rect2.maxX);
        double maxY = std::max(rect1.maxY, rect2.maxY);
        return Rect(minX, minY, maxX, maxY);
    }

    void adjustRect(RTreeNode* node, RTreeNode* newNode) {
        node->rect = mergeRects(node->rect, newNode->rect);
        if (node->children.empty()) {
            return;
        }
        for (RTreeNode* child : node->children) {
            adjustRect(child, newNode);
        }
    }

    void searchRect(RTreeNode* node, Rect rect, std::vector<Rect>& results) {
        if (node->children.empty()) {
            if (rectOverlap(node->rect, rect)) {
                results.push_back(node->rect);
            }
        } else {
            for (RTreeNode* child : node            ->children) {
                if (rectOverlap(child->rect, rect)) {
                    searchRect(child, rect, results);
                }
            }
        }
    }

    bool rectOverlap(Rect rect1, Rect rect2) {
        return (rect1.minX <= rect2.maxX && rect1.maxX >= rect2.minX && rect1.minY <= rect2.maxY && rect1.maxY >= rect2.minY);
    }
};

int main() {
    // create an R-tree with maximum 2 children per node
    RTree tree(2);

    // insert some rectangles
    tree.insert(Rect(0, 0, 1, 1));
    tree.insert(Rect(2, 2, 3, 3));
    tree.insert(Rect(4, 4, 5, 5));

    // search for rectangles that overlap with a query rectangle
    Rect query(2.5, 2.5, 4.5, 4.5);
    std::vector<Rect> results = tree.search(query);
    std::cout << "Found " << results.size() << " rectangles that overlap with (" 
              << query.minX << ", " << query.minY << ", " << query.maxX << ", " << query.maxY << ")" << std::endl;
    for (Rect result : results) {
        std::cout << "(" << result.minX << ", " << result.minY << ", " << result.maxX << ", " << result.maxY << ")" << std::endl;
    }

    return 0;
}

