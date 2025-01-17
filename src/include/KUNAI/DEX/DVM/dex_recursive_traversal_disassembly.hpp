/**
 * @file dex_recursive_traversal_disassembly.hpp
 * @author @Farenain
 * @brief Recursive traversal disassembly for DEX file.
 *        this in opposite to the linear sweep disassembly
 *        it will follow the control flow of the method.
 */

#ifndef DEX_RECURSIVE_TRAVERSAL_DISASSEMBLY_HPP
#define DEX_RECURSIVE_TRAVERSAL_DISASSEMBLY_HPP

#include <iostream>
#include <vector>
#include <istream>
#include <sstream>
#include <iterator>
#include <queue>

#include "KUNAI/DEX/DVM/dex_instructions.hpp"

namespace KUNAI {
    namespace DEX {
        class RecursiveTraversalDisassembler;

        using recursivetraversaldisassembler_t = std::shared_ptr<RecursiveTraversalDisassembler>;

        class RecursiveTraversalDisassembler
        {
        public:
            RecursiveTraversalDisassembler(dalvikopcodes_t dalvik_opcodes);
            ~RecursiveTraversalDisassembler() = default;

            std::map<std::uint64_t, instruction_t> disassembly(const std::vector<std::uint8_t>& byte_buffer, encodedmethod_t& method);
        private:
            void analyze_switch(std::map<std::uint64_t, instruction_t>& instrs, const std::vector<std::uint8_t>& byte_buffer, instruction_t& instruction, std::uint64_t instruction_index);

            std::queue<std::uint64_t> Q;
            dalvikopcodes_t dalvik_opcodes;
        };
    }
}

#endif