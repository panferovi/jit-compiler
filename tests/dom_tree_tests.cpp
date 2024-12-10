#include <gtest/gtest.h>
#include <sstream>
#include <iostream>

#include "ir/analysis.h"
#include "ir/basic_block.h"
#include "ir/common.h"
#include "ir/graph.h"
#include "ir/ir_builder.h"
#include "ir/instruction.h"

namespace compiler::tests {

using BBSet = std::set<ir::BasicBlock *>;

/**
 *  Graph:                                       Dominators Tree:
 *                              -----
 *                      --------| 0 |                            0
 *                      |       -----                            |
 *                      #                                        #
 *              true  -----  false                       --------1----
 *            --------| 1 |--------                      |   |       |
 *            |       -----       |                      #   #       #
 *            #                   #                      2   3   ----5----
 *          -----               -----                            |       |
 *  --------| 2 |       --------| 5 |--------                    #       #
 *  |       -----       |       -----       |                    4       6
 *  |                   #                   #
 *  |                 -----               -----
 *  |         --------| 4 |       --------| 6 |
 *  |         |       -----       |       -----
 *  ----------#--------------------
 *          -----
 *          | 3 |
 *          -----
 */
TEST(DOMINATOR_TREE, ExampleGraph1)
{
    auto graph = ir::Graph {};

    auto *bb0 = ir::BasicBlock::Create(&graph);
    auto *bb1 = ir::BasicBlock::Create(&graph);
    auto *bb2 = ir::BasicBlock::Create(&graph);
    auto *bb3 = ir::BasicBlock::Create(&graph);
    auto *bb4 = ir::BasicBlock::Create(&graph);
    auto *bb5 = ir::BasicBlock::Create(&graph);
    auto *bb6 = ir::BasicBlock::Create(&graph);

    bb0->SetTrueSuccessor(bb1);

    bb1->SetTrueSuccessor(bb2);
    bb1->SetFalseSuccessor(bb5);

    bb2->SetTrueSuccessor(bb3);

    bb4->SetTrueSuccessor(bb3);

    bb5->SetTrueSuccessor(bb4);
    bb5->SetFalseSuccessor(bb6);

    bb6->SetTrueSuccessor(bb3);

    ir::DominatorsTree tree {&graph};
    tree.Run();

    ASSERT(tree.GetDominators(bb0) == BBSet({}));
    ASSERT(tree.GetImmediateDominator(bb0) == nullptr);

    ASSERT(tree.GetDominators(bb1) == BBSet({bb0}));
    ASSERT(tree.GetImmediateDominator(bb1) == bb0);

    ASSERT(tree.GetDominators(bb2) == BBSet({bb0, bb1}));
    ASSERT(tree.GetImmediateDominator(bb2) == bb1);

    ASSERT(tree.GetDominators(bb3) == BBSet({bb0, bb1}));
    ASSERT(tree.GetImmediateDominator(bb3) == bb1);

    ASSERT(tree.GetDominators(bb5) == BBSet({bb0, bb1}));
    ASSERT(tree.GetImmediateDominator(bb5) == bb1);

    ASSERT(tree.GetDominators(bb4) == BBSet({bb0, bb1, bb5}));
    ASSERT(tree.GetImmediateDominator(bb4) == bb5);

    ASSERT(tree.GetDominators(bb6) == BBSet({bb0, bb1, bb5}));
    ASSERT(tree.GetImmediateDominator(bb6) == bb5);
}

/**
 *  Graph:                                                                      Dominators Tree:
 *                                                                      -----
 *                                                              --------| 0 |                   0
 *                                                              |       -----                   |
 *  ------------------------------------------------------------#                               #
 *  |                                                   true  -----  false                  ----1----
 *  |                                                 --------| 1 |--------                 |       |
 *  |                                                 |       -----       |                 #       #
 *  |                                                 |                   #                 2       9
 *  |                                                 |                 -----               |
 *  |                                                 ----------+-------| 9 |               #
 *  |                                                           |       -----               3
 *  |                                                           #--------                   |
 *  |                                                         -----     |                   #
 *  |                                                 --------| 2 |     |                   4
 *  |                                                 |       -----     |                   |
 *  |                                                 #                 |                   #
 *  |                                               -----               |                   5
 *  |                                       --------| 3 |----------------                   |
 *  |                                       |       -----                                   #
 *  |                                       #--------                                   ----6----
 *  |                                     -----     |                                   |       |
 *  |                             --------| 4 |     |                                   #       #
 *  |                             |       -----     |                                   7       8
 *  |                             #                 |                                           |
 *  |                           -----               |                                           #
 *  |                   --------| 5 |----------------                                           10
 *  |                   |       -----
 *  |                   #
 *  |                 -----
 *  |         --------| 6 |--------
 *  |         |       -----       |
 *  |         #                   #
 *  |       -----               -----
 *  --------| 7 |       --------| 8 |
 *          -----       |       -----
 *                      #
 *                    ------
 *                    | 10 |
 *                    ------
 */
TEST(DOMINATOR_TREE, ExampleGraph2)
{
    auto graph = ir::Graph {};

    auto *bb0 = ir::BasicBlock::Create(&graph);
    auto *bb1 = ir::BasicBlock::Create(&graph);
    auto *bb2 = ir::BasicBlock::Create(&graph);
    auto *bb3 = ir::BasicBlock::Create(&graph);
    auto *bb4 = ir::BasicBlock::Create(&graph);
    auto *bb5 = ir::BasicBlock::Create(&graph);
    auto *bb6 = ir::BasicBlock::Create(&graph);
    auto *bb7 = ir::BasicBlock::Create(&graph);
    auto *bb8 = ir::BasicBlock::Create(&graph);
    auto *bb9 = ir::BasicBlock::Create(&graph);
    auto *bb10 = ir::BasicBlock::Create(&graph);

    bb0->SetTrueSuccessor(bb1);

    bb1->SetTrueSuccessor(bb2);
    bb1->SetFalseSuccessor(bb9);

    bb2->SetTrueSuccessor(bb3);

    bb3->SetTrueSuccessor(bb4);
    bb3->SetFalseSuccessor(bb2);

    bb4->SetTrueSuccessor(bb5);

    bb5->SetTrueSuccessor(bb6);
    bb5->SetFalseSuccessor(bb4);

    bb6->SetTrueSuccessor(bb7);
    bb6->SetFalseSuccessor(bb8);

    bb7->SetTrueSuccessor(bb1);

    bb8->SetTrueSuccessor(bb10);

    ir::DominatorsTree tree {&graph};
    tree.Run();

    ASSERT(tree.GetDominators(bb0) == BBSet({}));
    ASSERT(tree.GetImmediateDominator(bb0) == nullptr);

    ASSERT(tree.GetDominators(bb1) == BBSet({bb0}));
    ASSERT(tree.GetImmediateDominator(bb1) == bb0);

    ASSERT(tree.GetDominators(bb2) == BBSet({bb0, bb1}));
    ASSERT(tree.GetImmediateDominator(bb2) == bb1);

    ASSERT(tree.GetDominators(bb3) == BBSet({bb0, bb1, bb2}));
    ASSERT(tree.GetImmediateDominator(bb3) == bb2);

    ASSERT(tree.GetDominators(bb4) == BBSet({bb0, bb1, bb2, bb3}));
    ASSERT(tree.GetImmediateDominator(bb4) == bb3);

    ASSERT(tree.GetDominators(bb5) == BBSet({bb0, bb1, bb2, bb3, bb4}));
    ASSERT(tree.GetImmediateDominator(bb5) == bb4);

    ASSERT(tree.GetDominators(bb6) == BBSet({bb0, bb1, bb2, bb3, bb4, bb5}));
    ASSERT(tree.GetImmediateDominator(bb6) == bb5);

    ASSERT(tree.GetDominators(bb7) == BBSet({bb0, bb1, bb2, bb3, bb4, bb5, bb6}));
    ASSERT(tree.GetImmediateDominator(bb7) == bb6);

    ASSERT(tree.GetDominators(bb8) == BBSet({bb0, bb1, bb2, bb3, bb4, bb5, bb6}));
    ASSERT(tree.GetImmediateDominator(bb8) == bb6);

    ASSERT(tree.GetDominators(bb10) == BBSet({bb0, bb1, bb2, bb3, bb4, bb5, bb6, bb8}));
    ASSERT(tree.GetImmediateDominator(bb10) == bb8);

    ASSERT(tree.GetImmediateDominatorFor(bb7, bb10) == bb6);
    ASSERT(tree.GetImmediateDominatorFor(bb5, bb3) == bb2);
    ASSERT(tree.GetImmediateDominatorFor(bb2, bb2) == bb1);
}

/**
 *  Graph:                                             Dominators Tree:
 *                                        -----
 *                                --------| 0 |                         0
 *                                |       -----                         |
 *  ------------------------------#                                     #
 *  |                     true  -----  false                    --------1------------
 *  |                   --------| 1 |-------+--------           |   |       |   |   |
 *  |                   |       -----       |       |           #   #       #   #   #
 *  |                   #                   #       |           2   3       4   6   8
 *  |                 -----               -----     |                       |
 *  |         --------| 4 |-------+-------| 2 |     |                       #
 *  |         |       -----       |       -----     |                       5
 *  |         #                   #                 |                       |
 *  |       -----               -----               |                       #
 *  --------| 5 |--------       | 3 |--------       |                       7
 *          -----       |       -----       |       |
 *                      #                   |       |
 *                    -----                 |       |
 *            --------| 7 |-------+----------       |
 *            |       -----       |                 |
 *            |                   #                 |
 *            |                 -----               |
 *            ----------+-------| 6 |----------------
 *                      |       -----
 *                      #
 *                    -----
 *                    | 8 |
 *                    -----
 */
TEST(DOMINATOR_TREE, ExampleGraph3)
{
    auto graph = ir::Graph {};

    auto *bb0 = ir::BasicBlock::Create(&graph);
    auto *bb1 = ir::BasicBlock::Create(&graph);
    auto *bb2 = ir::BasicBlock::Create(&graph);
    auto *bb3 = ir::BasicBlock::Create(&graph);
    auto *bb4 = ir::BasicBlock::Create(&graph);
    auto *bb5 = ir::BasicBlock::Create(&graph);
    auto *bb6 = ir::BasicBlock::Create(&graph);
    auto *bb7 = ir::BasicBlock::Create(&graph);
    auto *bb8 = ir::BasicBlock::Create(&graph);

    bb0->SetTrueSuccessor(bb1);

    bb1->SetTrueSuccessor(bb4);
    bb1->SetFalseSuccessor(bb2);

    bb2->SetTrueSuccessor(bb3);

    bb3->SetFalseSuccessor(bb6);

    bb4->SetTrueSuccessor(bb5);
    bb4->SetFalseSuccessor(bb3);

    bb5->SetTrueSuccessor(bb1);
    bb5->SetFalseSuccessor(bb7);

    bb6->SetTrueSuccessor(bb8);
    bb6->SetFalseSuccessor(bb2);

    bb7->SetTrueSuccessor(bb8);
    bb7->SetFalseSuccessor(bb6);

    ir::DominatorsTree tree {&graph};
    tree.Run();

    ASSERT(tree.GetDominators(bb0) == BBSet({}));
    ASSERT(tree.GetImmediateDominator(bb0) == nullptr);

    ASSERT(tree.GetDominators(bb1) == BBSet({bb0}));
    ASSERT(tree.GetImmediateDominator(bb1) == bb0);

    ASSERT(tree.GetDominators(bb2) == BBSet({bb0, bb1}));
    ASSERT(tree.GetImmediateDominator(bb2) == bb1);

    ASSERT(tree.GetDominators(bb3) == BBSet({bb0, bb1}));
    ASSERT(tree.GetImmediateDominator(bb3) == bb1);

    ASSERT(tree.GetDominators(bb4) == BBSet({bb0, bb1}));
    ASSERT(tree.GetImmediateDominator(bb4) == bb1);

    ASSERT(tree.GetDominators(bb5) == BBSet({bb0, bb1, bb4}));
    ASSERT(tree.GetImmediateDominator(bb5) == bb4);

    ASSERT(tree.GetDominators(bb6) == BBSet({bb0, bb1}));
    ASSERT(tree.GetImmediateDominator(bb6) == bb1);

    ASSERT(tree.GetDominators(bb7) == BBSet({bb0, bb1, bb4, bb5}));
    ASSERT(tree.GetImmediateDominator(bb7) == bb5);

    ASSERT(tree.GetDominators(bb8) == BBSet({bb0, bb1}));
    ASSERT(tree.GetImmediateDominator(bb8) == bb1);
}

}  // namespace compiler::tests
