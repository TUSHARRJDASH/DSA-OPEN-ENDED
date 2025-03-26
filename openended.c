#include <stdio.h>
#include <stdlib.h>

#define MAX_OBJECTS 4   // Maximum objects a node can hold before splitting
#define MAX_LEVELS 5    // Maximum depth of the quadtree

// Rectangle structure representing an area in 2D space
typedef struct {
    float x, y, width, height;
} Rectangle;

// Object structure representing an object with an ID and bounding box
typedef struct {
    int id;
    Rectangle bounds;
} Object;

// Quadtree node structure
typedef struct Quadtree {
    int level;
    Rectangle bounds;
    Object *objects[MAX_OBJECTS];
    int objectCount;
    struct Quadtree *nodes[4]; // Four child nodes
} Quadtree;

// Function to check if two rectangles overlap
int isOverlapping(Rectangle a, Rectangle b) {
    return !(a.x > b.x + b.width || a.x + a.width < b.x || 
             a.y > b.y + b.height || a.y + a.height < b.y);
}

// Create a new quadtree node
Quadtree* createQuadtree(int level, Rectangle bounds) {
    Quadtree *node = (Quadtree*)malloc(sizeof(Quadtree));
    node->level = level;
    node->bounds = bounds;
    node->objectCount = 0;
    for (int i = 0; i < 4; i++) node->nodes[i] = NULL;
    return node;
}

// Subdivide a quadtree node into four child nodes
void subdivide(Quadtree *node) {
    float subWidth = node->bounds.width / 2.0f;
    float subHeight = node->bounds.height / 2.0f;
    float x = node->bounds.x;
    float y = node->bounds.y;

    node->nodes[0] = createQuadtree(node->level + 1, (Rectangle){x, y, subWidth, subHeight});            // NW
    node->nodes[1] = createQuadtree(node->level + 1, (Rectangle){x + subWidth, y, subWidth, subHeight}); // NE
    node->nodes[2] = createQuadtree(node->level + 1, (Rectangle){x, y + subHeight, subWidth, subHeight}); // SW
    node->nodes[3] = createQuadtree(node->level + 1, (Rectangle){x + subWidth, y + subHeight, subWidth, subHeight}); // SE
}

// Get the index of the quadrant where the object belongs
int getIndex(Quadtree *node, Rectangle bounds) {
    int index = -1;
    float verticalMidpoint = node->bounds.x + node->bounds.width / 2.0f;
    float horizontalMidpoint = node->bounds.y + node->bounds.height / 2.0f;

    int topQuadrant = (bounds.y < horizontalMidpoint && bounds.y + bounds.height < horizontalMidpoint);
    int bottomQuadrant = (bounds.y > horizontalMidpoint);

    if (bounds.x < verticalMidpoint && bounds.x + bounds.width < verticalMidpoint) {
        if (topQuadrant) index = 0;  // NW
        else if (bottomQuadrant) index = 2; // SW
    } else if (bounds.x > verticalMidpoint) {
        if (topQuadrant) index = 1;  // NE
        else if (bottomQuadrant) index = 3; // SE
    }

    return index;
}

// Insert an object into the quadtree
void insert(Quadtree *node, Object *object) {
    if (node->nodes[0] != NULL) {
        int index = getIndex(node, object->bounds);

        if (index != -1) {
            insert(node->nodes[index], object);
            return;
        }
    }

    node->objects[node->objectCount++] = object;

    if (node->objectCount > MAX_OBJECTS && node->level < MAX_LEVELS) {
        if (node->nodes[0] == NULL) subdivide(node);

        int i = 0;
        while (i < node->objectCount) {
            int index = getIndex(node, node->objects[i]->bounds);
            if (index != -1) {
                insert(node->nodes[index], node->objects[i]);
                for (int j = i; j < node->objectCount - 1; j++) {
                    node->objects[j] = node->objects[j + 1];
                }
                node->objectCount--;
            } else {
                i++;
            }
        }
    }
}

// Retrieve potential colliders for a given object
void retrieve(Quadtree *node, Object *object, Object **returnObjects, int *returnCount) {
    int index = getIndex(node, object->bounds);
    if (index != -1 && node->nodes[0] != NULL) {
        retrieve(node->nodes[index], object, returnObjects, returnCount);
    }

    for (int i = 0; i < node->objectCount; i++) {
        returnObjects[(*returnCount)++] = node->objects[i];
    }
}

// Free the quadtree
void freeQuadtree(Quadtree *node) {
    for (int i = 0; i < 4; i++) {
        if (node->nodes[i] != NULL) {
            freeQuadtree(node->nodes[i]);
        }
    }
    free(node);
}

// Example usage
int main() {
    Rectangle worldBounds = {0, 0, 100, 100};
    Quadtree *root = createQuadtree(0, worldBounds);

    Object obj1 = {1, {10, 10, 5, 5}};
    Object obj2 = {2, {15, 15, 5, 5}};
    Object obj3 = {3, {80, 80, 5, 5}};

    insert(root, &obj1);
    insert(root, &obj2);
    insert(root, &obj3);

    Object *potentialColliders[10];
    int count = 0;
    retrieve(root, &obj1, potentialColliders, &count);

    printf("Potential colliders for Object 1:\n");
    for (int i = 0; i < count; i++) {
        printf("Object ID: %d\n", potentialColliders[i]->id);
    }

    freeQuadtree(root);
    return 0;
}
