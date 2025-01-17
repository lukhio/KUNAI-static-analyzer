#include "KUNAI/DEX/parser/dex_classes.hpp"

namespace KUNAI
{
    namespace DEX
    {
        /***
         * ClassDataItem
         */

        ClassDataItem::ClassDataItem(std::ifstream &input_file,
                                     std::uint64_t file_size,
                                     dexfields_t& dex_fields,
                                     dexmethods_t& dex_methods,
                                     dextypes_t& dex_types)
        {
            auto current_offset = input_file.tellg();

            std::uint64_t static_fields_size,
                instance_fields_size,
                direct_methods_size,
                virtual_methods_size;
            std::uint64_t static_field = 0,
                          instance_field = 0,
                          direct_method = 0,
                          virtual_method = 0,
                          access_flags,
                          code_off;

            static_fields_size = KUNAI::read_uleb128(input_file);
            instance_fields_size = KUNAI::read_uleb128(input_file);
            direct_methods_size = KUNAI::read_uleb128(input_file);
            virtual_methods_size = KUNAI::read_uleb128(input_file);

            for (size_t i = 0; i < static_fields_size; i++)
            {
                // value as it's field_idx_diff, needs to be incremented
                // taking care of previous value
                // static_field = static_field_read + prev
                static_field += KUNAI::read_uleb128(input_file);

                if (static_field >= dex_fields->get_number_of_fields())
                    throw exceptions::IncorrectFieldId("Error reading ClassDataItem static_field out of field bound");

                access_flags = KUNAI::read_uleb128(input_file);

                static_fields[static_field] = std::make_shared<EncodedField>(dex_fields->get_field_id_by_order(static_field), access_flags);
            }

            for (size_t i = 0; i < instance_fields_size; i++)
            {
                instance_field += KUNAI::read_uleb128(input_file);

                if (instance_field >= dex_fields->get_number_of_fields())
                    throw exceptions::IncorrectFieldId("Error reading ClassDataItem instance_field out of field bound");

                access_flags = KUNAI::read_uleb128(input_file);
                instance_fields[instance_field] = std::make_shared<EncodedField>(dex_fields->get_field_id_by_order(instance_field), access_flags);
            }

            for (size_t i = 0; i < direct_methods_size; i++)
            {
                direct_method += KUNAI::read_uleb128(input_file);

                if (direct_method >= dex_methods->get_number_of_methods())
                    throw exceptions::IncorrectMethodId("Error reading ClassDataItem direct_method out of method bound");

                access_flags = KUNAI::read_uleb128(input_file);

                code_off = KUNAI::read_uleb128(input_file);

                direct_methods[direct_method] = std::make_shared<EncodedMethod>(dex_methods->get_method_by_order(direct_method), access_flags, code_off, input_file, file_size, dex_types);
                methods.push_back(direct_methods[direct_method]);
            }

            for (size_t i = 0; i < virtual_methods_size; i++)
            {
                virtual_method += KUNAI::read_uleb128(input_file);

                if (virtual_method >= dex_methods->get_number_of_methods())
                    throw exceptions::IncorrectMethodId("Error reading ClassDataItem virtual_method out of method bound");

                access_flags = KUNAI::read_uleb128(input_file);

                code_off = KUNAI::read_uleb128(input_file);

                virtual_methods[virtual_method] = std::make_shared<EncodedMethod>(dex_methods->get_method_by_order(virtual_method), access_flags, code_off, input_file, file_size, dex_types);
                methods.push_back(virtual_methods[virtual_method]);
            }

            input_file.seekg(current_offset);
        }

        encodedfield_t ClassDataItem::get_static_field_by_id(std::uint64_t id)
        {
            auto it = static_fields.find(id);

            if (it != static_fields.end())
                return it->second;

            return nullptr;
        }

        encodedfield_t ClassDataItem::get_static_field_by_pos(std::uint64_t pos)
        {
            if (pos >= static_fields.size())
                return nullptr;

            auto it = static_fields.begin();
            while (pos-- != 0)
                it++;
            return it->second;
        }

        encodedfield_t ClassDataItem::get_instance_field_by_id(std::uint64_t id)
        {
            auto it = instance_fields.find(id);

            if (it != instance_fields.end())
                return it->second;

            return nullptr;
        }

        encodedfield_t ClassDataItem::get_instance_field_by_pos(std::uint64_t pos)
        {
            if (pos >= instance_fields.size())
                return nullptr;

            auto it = instance_fields.begin();
            while (pos-- != 0)
                it++;
            return it->second;
        }

        std::vector<encodedfield_t> ClassDataItem::get_fields()
        {
            std::vector<encodedfield_t> fields;

            if (!static_fields.empty())
            {
                for (auto it = static_fields.begin(); it != static_fields.end(); it++)
                {
                    fields.push_back(it->second);
                }
            }

            if (!instance_fields.empty())
            {
                for (auto it = instance_fields.begin(); it != instance_fields.end(); it++)
                {
                    fields.push_back(it->second);
                }
            }
            return fields;
        }

        encodedmethod_t ClassDataItem::get_direct_method_by_id(std::uint64_t id)
        {
            auto it = direct_methods.find(id);

            if (it != direct_methods.end())
                return it->second;

            return nullptr;
        }

        encodedmethod_t ClassDataItem::get_direct_method_by_pos(std::uint64_t pos)
        {
            if (pos >= direct_methods.size())
                return nullptr;

            auto it = direct_methods.begin();
            while (pos-- != 0)
                it++;
            return it->second;
        }

        encodedmethod_t ClassDataItem::get_virtual_method_by_id(std::uint64_t id)
        {
            auto it = virtual_methods.find(id);

            if (it != virtual_methods.end())
                return it->second;

            return nullptr;
        }

        encodedmethod_t ClassDataItem::get_virtual_method_by_pos(std::uint64_t pos)
        {
            if (pos >= virtual_methods.size())
                return nullptr;

            auto it = virtual_methods.begin();
            while (pos-- != 0)
                it++;
            return it->second;
        }

        /***
         * ClassDef
         */
        ClassDef::ClassDef(classdef_t class_def,
                           dexstrings_t& dex_str,
                           dextypes_t& dex_types,
                           dexfields_t& dex_fields,
                           dexmethods_t& dex_methods,
                           std::ifstream &input_file,
                           std::uint64_t file_size)
        {
            this->class_idx[class_def.class_idx] = std::dynamic_pointer_cast<Class>(dex_types->get_type_from_order(class_def.class_idx));
            if (class_def.superclass_idx == DVMTypes::NO_INDEX)
                this->superclass_idx[class_def.superclass_idx] = nullptr;
            else
                this->superclass_idx[class_def.superclass_idx] = std::dynamic_pointer_cast<Class>(dex_types->get_type_from_order(class_def.superclass_idx));

            if (class_def.source_file_idx == DVMTypes::NO_INDEX)
                this->source_file_idx[class_def.source_file_idx] = nullptr;
            else
                this->source_file_idx[class_def.source_file_idx] = dex_str->get_string_from_order(class_def.source_file_idx);
            this->access_flag = static_cast<DVMTypes::ACCESS_FLAGS>(class_def.access_flags);
            this->interfaces_off = class_def.interfaces_off;
            this->annotations_off = class_def.annotations_off;
            this->classess_off = class_def.class_data_off;
            this->static_values_off = class_def.static_values_off;

            if (!parse_class_defs(input_file, file_size, dex_str, dex_types, dex_fields, dex_methods))
                throw exceptions::ParserReadingException("Error reading DEX ClassDef");
        }

        bool ClassDef::parse_class_defs(std::ifstream &input_file,
                                        std::uint64_t file_size,
                                        dexstrings_t& dex_str,
                                        dextypes_t& dex_types,
                                        dexfields_t& dex_fields,
                                        dexmethods_t& dex_methods)
        {
            auto logger = LOGGER::logger();

            auto current_offset = input_file.tellg();
            size_t i;
            std::uint32_t size;
            std::uint16_t interface;

#ifdef DEBUG
            logger->debug("ClassDef parsing values from class");
#endif

            // parse first the interfaces
            if (interfaces_off > 0)
            {
                input_file.seekg(interfaces_off);

                if (!KUNAI::read_data_file<std::uint32_t>(size, sizeof(std::uint32_t), input_file))
                    return false;

#ifdef DEBUG
                logger->debug("Parsing interfaces with offset {} and size {}", interfaces_off, size);
#endif

                for (i = 0; i < size; i++)
                {
                    if (!KUNAI::read_data_file<std::uint16_t>(interface, sizeof(std::uint16_t), input_file))
                        return false;

                    if (interface >= dex_types->get_number_of_types())
                        throw exceptions::IncorrectTypeId("Error reading DEX ClassDefs interface out of type bound");

                    interfaces[interface] = std::dynamic_pointer_cast<Class>(dex_types->get_type_from_order(interface));
                }
            }

            // parse annotations
            if (annotations_off > 0)
            {
                input_file.seekg(annotations_off);

#ifdef DEBUG
                logger->debug("Parsing AnnotationsDirectoryItem in offset {}", annotations_off);
#endif

                annotation_directory_item = std::make_shared<AnnotationsDirectoryItem>(input_file);
            }

            // parse class data
            if (classess_off > 0)
            {
                input_file.seekg(classess_off);

#ifdef DEBUG
                logger->debug("Parsing ClassDataItem in offset {}", classess_off);
#endif

                class_data_items = std::make_shared<ClassDataItem>(input_file, file_size, dex_fields, dex_methods, dex_types);
            }

            /**
            if (static_values_off > 0)
            {
                input_file.seekg(static_values_off);

                static_values = std::make_shared<EncodedArrayItem>(input_file);
            }
            */

            input_file.seekg(current_offset);
            return true;
        }

        class_t ClassDef::get_interface_by_class_id(std::uint16_t id)
        {
            auto it = interfaces.find(id);

            if (it != interfaces.end())
                return it->second;

            return nullptr;
        }

        class_t ClassDef::get_interface_by_pos(std::uint64_t pos)
        {
            if (pos >= interfaces.size())
                return nullptr;
            auto it = interfaces.begin();
            while (pos-- != 0)
                it++;
            return it->second;
        }

        /***
         * DexClasses
         */

        DexClasses::DexClasses(std::ifstream &input_file,
                               std::uint64_t file_size,
                               std::uint32_t number_of_classes,
                               std::uint32_t offset,
                               dexstrings_t& dex_str,
                               dextypes_t& dex_types,
                               dexfields_t& dex_fields,
                               dexmethods_t& dex_methods) : number_of_classes(number_of_classes),
                                                                          offset(offset),
                                                                          dex_str(dex_str),
                                                                          dex_types(dex_types),
                                                                          dex_fields(dex_fields),
                                                                          dex_methods(dex_methods)
        {
            if (!parse_classes(input_file, file_size))
                throw exceptions::ParserReadingException("Error reading DEX classes");
        }

        DexClasses::~DexClasses()
        {
            if (!class_defs.empty())
            {
                class_defs.clear();
            }
        }

        classdef_t DexClasses::get_class_by_pos(std::uint64_t pos)
        {
            if (pos >= class_defs.size())
                return nullptr;
            return class_defs[pos];
        }

        bool DexClasses::parse_classes(std::ifstream &input_file, std::uint64_t file_size)
        {
            auto logger = LOGGER::logger();

            auto current_offset = input_file.tellg();
            size_t i;
            ClassDef::classdef_t class_def_struct;
            classdef_t class_def;

            input_file.seekg(offset);

#ifdef DEBUG
            logger->debug("DexClasses start parsing at offset {} and size {}", offset, number_of_classes);
#endif

            for (i = 0; i < number_of_classes; i++)
            {
                if (!KUNAI::read_data_file<ClassDef::classdef_t>(class_def_struct, sizeof(ClassDef::classdef_t), input_file))
                    return false;

                if (class_def_struct.class_idx >= dex_types->get_number_of_types())
                {
                    logger->error("Error reading DEX classes class_idx out of type bound ({} >= {})", class_def_struct.class_idx, dex_types->get_number_of_types());
                    throw exceptions::IncorrectTypeId("Error reading DEX classes class_idx out of type bound");
                }

                if (class_def_struct.access_flags > DVMTypes::ACC_DECLARED_SYNCHRONIZED)
                {
                    logger->error("Error reading DEX classes access_flags incorrect value ({})", class_def_struct.access_flags);
                    throw exceptions::IncorrectValue("Error reading DEX classes access_flags incorrect value");
                }

                if ((class_def_struct.superclass_idx != DVMTypes::NO_INDEX) && (class_def_struct.superclass_idx >= dex_types->get_number_of_types()))
                {
                    logger->error("Error reading DEX classes superclass_idx out of type bound ({} >= {})", class_def_struct.superclass_idx, dex_types->get_number_of_types());
                    throw exceptions::IncorrectTypeId("Error reading DEX classes superclass_idx out of type bound");
                }

                if (class_def_struct.interfaces_off > file_size)
                {
                    logger->error("Error reading DEX classes interfaces_off out of file bound ({} > {})", class_def_struct.interfaces_off, file_size);
                    throw exceptions::OutOfBoundException("Error reading DEX classes interfaces_off out of file bound");
                }

                if ((class_def_struct.source_file_idx != DVMTypes::NO_INDEX) && (class_def_struct.source_file_idx >= dex_str->get_number_of_strings()))
                {
                    logger->error("Error reading DEX classes source_file_idx out of string bound ({} >= {})", class_def_struct.source_file_idx, dex_str->get_number_of_strings());
                    throw exceptions::IncorrectStringId("Error reading DEX classes source_file_idx out of string bound");
                }

                if (class_def_struct.annotations_off > file_size)
                {
                    logger->error("Error reading DEX classes annotations_off out of file bound ({} > {})", class_def_struct.annotations_off, file_size);
                    throw exceptions::OutOfBoundException("Error reading DEX classes annotations_off out of file bound");
                }

                if (class_def_struct.class_data_off > file_size)
                {
                    logger->error("Error reading DEX classes class_data_off out of file bound ({} > {})", class_def_struct.class_data_off, file_size);
                    throw exceptions::OutOfBoundException("Error reading DEX classes class_data_off out of file bound");
                }

                if (class_def_struct.static_values_off > file_size)
                {
                    logger->error("Error reading DEX classes static_values_off out of file bound ({} > {})", class_def_struct.static_values_off, file_size);
                    throw exceptions::OutOfBoundException("Error reading DEX classes static_values_off out of file bound");
                }

                class_def = std::make_shared<ClassDef>(class_def_struct, dex_str, dex_types, dex_fields, dex_methods, input_file, file_size);
                class_defs.push_back(class_def);

#ifdef DEBUG
                logger->debug("parsed class_def number {}", i);
#endif
            }

            input_file.seekg(current_offset);

            logger->info("DexClasses parsing correct");

            return true;
        }

        std::ostream &operator<<(std::ostream &os, const DexClasses &entry)
        {
            size_t i = 0;
            os << std::hex;
            os << std::setw(30) << std::left << std::setfill(' ') << "=========== DEX Classes ===========" << std::endl;

            for (auto class_def : entry.class_defs)
            {
                os << "Class (" << i++ << "):" << std::endl;
                if (class_def->get_class_idx())
                    os << "\tClass idx: " << class_def->get_class_idx()->get_name() << std::endl;
                os << "\tAccess Flags: " << class_def->get_access_flags() << std::endl;
                if (class_def->get_superclass_idx())
                    os << "\tSuperclass: " << class_def->get_superclass_idx()->get_name() << std::endl;
                if (class_def->get_source_file_idx())
                    os << "\tSource File: " << *class_def->get_source_file_idx() << std::endl;

                if (class_def->get_number_of_interfaces() > 0)
                    os << "\tInterfaces: " << std::endl;
                for (size_t j = 0; j < class_def->get_number_of_interfaces(); j++)
                {
                    os << "\t\tInterface(" << j << "):" << class_def->get_interface_by_pos(j)->get_name() << std::endl;
                }

                classdataitem_t class_data_item = class_def->get_class_data();

                if (class_data_item)
                {
                    os << "\tClassDataItem:" << std::endl;

                    if (class_data_item->get_number_of_static_fields() > 0)
                        os << "\t\tStatic fields:" << std::endl;
                    for (size_t j = 0; j < class_data_item->get_number_of_static_fields(); j++)
                    {
                        os << "\t\t\tStatic field(" << j << "): " << std::endl;
                        os << "\t\t\t\tAccess flags: " << class_data_item->get_static_field_by_pos(j)->get_access_flags() << std::endl;
                        os << "\t\t\t\tField: " << *class_data_item->get_static_field_by_pos(j)->get_field();
                    }

                    if (class_data_item->get_number_of_instance_fields() > 0)
                        os << "\t\tInstance fields:" << std::endl;
                    for (size_t j = 0; j < class_data_item->get_number_of_instance_fields(); j++)
                    {
                        os << "\t\t\tInstance field(" << j << "): " << std::endl;
                        os << "\t\t\t\tAccess flags: " << class_data_item->get_instance_field_by_pos(j)->get_access_flags() << std::endl;
                        os << "\t\t\t\tField: " << *class_data_item->get_instance_field_by_pos(j)->get_field();
                    }

                    if (class_data_item->get_number_of_direct_methods() > 0)
                        os << "\t\tDirect methods: " << std::endl;
                    for (size_t j = 0; j < class_data_item->get_number_of_direct_methods(); j++)
                    {
                        os << "\t\t\tDirect method(" << j << "): " << std::endl;
                        os << "\t\t\t\tAccess flags: " << class_data_item->get_direct_method_by_pos(j)->get_access_flags() << std::endl;
                        os << "\t\t\t\tCode offset: " << class_data_item->get_direct_method_by_pos(j)->get_code_offset() << std::endl;
                        os << "\t\t\t\tMethod: " << *class_data_item->get_direct_method_by_pos(j)->get_method() << std::endl;
                    }

                    if (class_data_item->get_number_of_virtual_methods() > 0)
                        os << "\t\tVirtual methods: " << std::endl;
                    for (size_t j = 0; j < class_data_item->get_number_of_virtual_methods(); j++)
                    {
                        os << "\t\t\tVirtual method(" << j << "): " << std::endl;
                        os << "\t\t\t\tAccess flags: " << class_data_item->get_virtual_method_by_pos(j)->get_access_flags() << std::endl;
                        os << "\t\t\t\tCode offset: " << class_data_item->get_virtual_method_by_pos(j)->get_code_offset() << std::endl;
                        os << "\t\t\t\tMethod: " << *class_data_item->get_virtual_method_by_pos(j)->get_method() << std::endl;
                    }
                }
            }

            return os;
        }

    }
}