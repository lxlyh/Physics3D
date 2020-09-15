#include "testsMain.h"

#include "compare.h"
#include "generators.h"
#include "../physics/misc/toString.h"
#include "../physics/misc/validityHelper.h"

#include "../physics/datastructures/boundsTree.h"

TEST_CASE(testBoundsTreeGenerationValid) {
	for(int iter = 0; iter < 100; iter++) {
		BoundsTree<BasicBounded> tree = generateBoundsTree();
		treeValidCheck(tree);
		ASSERT_TRUE(true);
	}
}

TEST_CASE(testAddObjToBoundsTree) {
	for(int iter = 0; iter < 100; iter++) {
		BoundsTree<BasicBounded> tree = generateBoundsTree();
		for(int jiter = 0; jiter < 10; jiter++) {
			BasicBounded* newObj = new BasicBounded{generateBounds()};
			ASSERT_FALSE(tree.contains(newObj));
			tree.add(newObj);
			ASSERT_TRUE(tree.contains(newObj));
			treeValidCheck(tree);
		}
	}
}
TEST_CASE(testRemoveObjFromBoundsTree) {
	for(int iter = 0; iter < 100; iter++) {
		BoundsTree<BasicBounded> tree = generateBoundsTree();
		while(!tree.isEmpty()) {
			BasicBounded* selectedObj = getRandomObjectFromTree(tree);
			ASSERT_TRUE(tree.contains(selectedObj));
			tree.remove(selectedObj);
			ASSERT_FALSE(tree.contains(selectedObj));
			treeValidCheck(tree);
		}
	}
}