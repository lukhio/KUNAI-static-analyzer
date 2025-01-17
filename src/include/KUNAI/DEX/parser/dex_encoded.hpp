/***
 * @file dex_encoded.hpp
 * @author @Farenain
 *
 * @brief All the different Encoded types from
 *        DEX format, I think is a good way
 *        to separate them from other logic of
 *        the parser.
 */

#ifndef DEX_ENCODED_HPP
#define DEX_ENCODED_HPP

#include <iostream>
#include <vector>
#include <memory>
#include <map>

#include "KUNAI/DEX/parser/dex_fields.hpp"
#include "KUNAI/DEX/parser/dex_methods.hpp"
#include "KUNAI/DEX/DVM/dex_dvm_types.hpp"

namespace KUNAI
{
    namespace DEX
    {
        class EncodedValue;
        class EncodedAnnotation;

        using encodedvalue_t = std::shared_ptr<EncodedValue>;
        using encodedannotation_t = std::shared_ptr<EncodedAnnotation>;
        
        class EncodedValue
        {
        public:
            EncodedValue(std::ifstream &input_file);
            ~EncodedValue();

            const std::vector<std::uint8_t> &get_values() const
            {
                return values;
            }

            const std::vector<encodedvalue_t> &get_array() const
            {
                return array;
            }

            encodedannotation_t get_annotation()
            {
                return annotation;
            }

        private:
            std::vector<std::uint8_t> values;
            std::vector<encodedvalue_t> array;
            encodedannotation_t annotation;
        };

        class EncodedArray;

        using encodedarray_t = std::shared_ptr<EncodedArray>;

        class EncodedArray
        {
        public:
            EncodedArray(std::ifstream &input_file);
            ~EncodedArray();

            std::uint64_t get_size()
            {
                return size;
            }

            const std::vector<encodedvalue_t> &get_values() const
            {
                return values;
            }

        private:
            std::uint64_t size;
            std::vector<encodedvalue_t> values;
        };

        class EncodedArrayItem;

        using encodedarrayitem_t = std::shared_ptr<EncodedArrayItem>;

        class EncodedArrayItem
        {
        public:
            EncodedArrayItem(std::ifstream &input_file);
            ~EncodedArrayItem() = default;

            encodedarray_t get_encoded_array()
            {
                return array;
            }

        private:
            encodedarray_t array;
        };

        class EncodedField;

        using encodedfield_t = std::shared_ptr<EncodedField>;

        class EncodedField
        {
        public:
            EncodedField(fieldid_t field_idx, std::uint64_t access_flags);
            ~EncodedField() = default;

            fieldid_t get_field() const
            {
                return field_idx;
            }

            DVMTypes::ACCESS_FLAGS get_access_flags()
            {
                return access_flags;
            }

        private:
            fieldid_t field_idx;
            DVMTypes::ACCESS_FLAGS access_flags;
        };

        class EncodedTypePair;

        using encodedtypepair_t = std::shared_ptr<EncodedTypePair>;

        class EncodedTypePair
        {
        public:
            EncodedTypePair(std::uint64_t type_idx,
                            std::uint64_t addr,
                            dextypes_t dex_types);

            ~EncodedTypePair() = default;

            type_t get_exception_type();

            std::uint64_t get_exception_handler_addr()
            {
                return addr;
            }

        private:
            std::map<std::uint64_t, type_t> type_idx; // type of the exception to catch
            std::uint64_t addr;                       // bytecode address of associated exception handler
        };

        class EncodedCatchHandler;

        using encodedcatchhandler_t = std::shared_ptr<EncodedCatchHandler>;

        class EncodedCatchHandler
        {
        public:
            EncodedCatchHandler(std::ifstream &input_file,
                                std::uint64_t file_size,
                                dextypes_t dex_types);

            ~EncodedCatchHandler();

            bool has_explicit_typed_catches();

            std::uint64_t get_size_of_handlers()
            {
                return handlers.size();
            }

            encodedtypepair_t get_handler_by_pos(std::uint64_t pos);

            std::uint64_t get_catch_all_addr()
            {
                return catch_all_addr;
            }

            std::uint64_t get_offset()
            {
                return offset;
            }

        private:
            bool parse_encoded_type_pairs(std::ifstream &input_file,
                                          std::uint64_t file_size,
                                          dextypes_t dex_types);

            std::uint64_t offset;
            std::int64_t encoded_type_pair_size;
            std::vector<encodedtypepair_t> handlers;
            std::uint64_t catch_all_addr;
        };

        class TryItem;

        using tryitem_t = std::shared_ptr<TryItem>;

        class TryItem
        {
        public:
            struct try_item_struct_t
            {
                std::uint32_t start_addr;  // start address of block of code covered by this entry.
                                           // Count of 16-bit code units to start of first.
                std::uint16_t insn_count;  // number of 16-bit code units covered by this entry.
                std::uint16_t handler_off; // offset in bytes from starts of associated encoded_catch_handler_list to
                                           // encoded_catch_handler for this entry.
            };

            TryItem(try_item_struct_t try_item_struct);
            ~TryItem() = default;

            std::uint32_t get_start_addr()
            {
                return try_item_struct.start_addr;
            }

            std::uint16_t get_insn_count()
            {
                return try_item_struct.insn_count;
            }

            std::uint16_t get_handler_off()
            {
                return try_item_struct.handler_off;
            }

        private:
            try_item_struct_t try_item_struct;
        };

        class CodeItemStruct;

        using codeitemstruct_t = std::shared_ptr<CodeItemStruct>;

        class CodeItemStruct
        {
        public:
            struct code_item_struct_t
            {
                std::uint16_t registers_size; // number of registers used in the code
                std::uint16_t ins_size;       // number of words of incoming arguments to the method
                std::uint16_t outs_size;      // number of words of outgoing argument space required by code for method invocation.
                std::uint16_t tries_size;     // number of try_items, it can be zero.
                std::uint32_t debug_info_off; // offset to debug_info_item
                std::uint32_t insns_size;     // size of instructions list, in 16-bit code units
            };

            CodeItemStruct(std::ifstream &input_file,
                           std::uint64_t file_size,
                           code_item_struct_t code_item,
                           dextypes_t dex_types);

            ~CodeItemStruct();

            std::uint16_t get_number_of_registers_in_code()
            {
                return code_item.registers_size;
            }

            std::uint16_t get_number_of_incoming_arguments()
            {
                return code_item.ins_size;
            }

            std::uint16_t get_number_of_outgoing_arguments()
            {
                return code_item.outs_size;
            }

            std::uint16_t get_number_of_try_items()
            {
                return code_item.tries_size;
            }

            tryitem_t get_try_item_by_pos(std::uint64_t pos);

            // raw byte instructions
            std::uint16_t get_number_of_raw_instructions()
            {
                return code_item.insns_size;
            }

            std::uint16_t get_raw_instruction_by_pos(std::uint16_t pos);

            std::vector<std::uint8_t> &get_all_raw_instructions()
            {
                return instructions_raw;
            }

            std::uint64_t get_encoded_catch_handler_offset()
            {
                return encoded_catch_handler_list_offset;
            }

            std::uint64_t get_encoded_catch_handler_list_size()
            {
                return encoded_catch_handler_list.size();
            }

            encodedcatchhandler_t get_encoded_catch_handler_by_pos(std::uint64_t pos);

        private:
            bool parse_code_item_struct(std::ifstream &input_file, std::uint64_t file_size, dextypes_t dex_types);

            code_item_struct_t code_item;
            std::vector<std::uint8_t> instructions_raw;
            std::vector<tryitem_t> try_items;
            std::uint64_t encoded_catch_handler_list_offset;
            std::vector<encodedcatchhandler_t> encoded_catch_handler_list;
        };

        class EncodedMethod;

        using encodedmethod_t = std::shared_ptr<EncodedMethod>;

        class EncodedMethod
        {
        public:
            EncodedMethod(methodid_t method_id,
                          std::uint64_t access_flags,
                          std::uint64_t code_off,
                          std::ifstream &input_file,
                          std::uint64_t file_size,
                          dextypes_t dex_types);
            ~EncodedMethod() = default;

            methodid_t get_method() const
            {
                return method_id;
            }

            DVMTypes::ACCESS_FLAGS get_access_flags()
            {
                return access_flags;
            }

            std::uint64_t get_code_offset()
            {
                return code_off;
            }

            codeitemstruct_t get_code_item()
            {
                return code_item;
            }

            std::string full_name();

        private:
            bool parse_code_item(std::ifstream &input_file, std::uint64_t file_size, dextypes_t dex_types);

            methodid_t method_id;
            DVMTypes::ACCESS_FLAGS access_flags;
            std::uint64_t code_off;
            codeitemstruct_t code_item;
        };

        class AnnotationElement;

        using annotationelement_t = std::shared_ptr<AnnotationElement>;

        class AnnotationElement
        {
        public:
            AnnotationElement(std::ifstream& input_file);
            ~AnnotationElement() = default;

            std::uint64_t get_name_idx()
            {
                return name_idx;
            }

            encodedvalue_t get_value()
            {
                return value;
            }
        private:
            std::uint64_t name_idx;
            encodedvalue_t value;
        };


        class EncodedAnnotation
        {
        public:
            EncodedAnnotation(std::ifstream& input_file);
            ~EncodedAnnotation();

            std::uint64_t get_type_idx()
            {
                return type_idx;
            }

            std::uint64_t get_number_of_elements()
            {
                return size;
            }

            const std::vector<annotationelement_t>& get_elements() const
            {
                return elements;
            }
        private:
            std::uint64_t type_idx;
            std::uint64_t size;
            std::vector<annotationelement_t> elements;
        };
    }
}

#endif