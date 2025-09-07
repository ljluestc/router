#include <gtest/gtest.h>
#include <iostream>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "Running Router Simulator Tests" << std::endl;
    std::cout << "==============================" << std::endl;
    
    return RUN_ALL_TESTS();
}
