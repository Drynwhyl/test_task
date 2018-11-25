#pragma once

#include <list>
#include <regex>
#include <string>
#include <fstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace crashparser
{
    namespace pt = boost::property_tree;

    /**
     * Represents parsed exception data
     */
    struct exception_data
    {
        std::string line;
        std::string type;
        std::string message;
    };

    template<template<typename, typename> typename Container = std::list>
    class crashparser
    {
        Container<exception_data, std::allocator<crashparser>> data;

    public:

        /**
         * Opens file with given file name, parses it's content
         * and writes parsed data to output stream
         */
        crashparser(std::string const& filename, std::ostream& output_stream)
            : filename(filename)
        {
            std::ifstream file { filename.data() };

            if (!file.good()) throw std::runtime_error { strerror(errno) };

            parse(file);
            write(output_stream);
        }

        /**
         * Writes parsed exception data to output_stream output_stream using json format
         */
        void write(std::ostream& output_stream)
        {
            pt::write_json(output_stream, root);
        }

    private:

        std::string filename;
        pt::ptree root;

        /**
         * Reads log text from input_stream and inserts parsed data using inserter iterator
         */
        void parse(std::istream& input_stream)
        {
            static std::regex const exception_pattern {
                    R"REGEX(File ".+", line (\d+), in .+\s+(?:.*\s+)?(\w+): ([^\n]+)(?:\n|$))REGEX"
            };

            std::string in { std::istreambuf_iterator { input_stream }, {} };
            auto inserter = std::inserter(data, std::begin(data));

            for (
                    std::sregex_iterator it { in.begin(), in.end(), exception_pattern }, it_end;
                    it != it_end;
                    ++it
                    )
            {
                auto& match_results = *it;
                *inserter++ = { match_results[1], match_results[2], match_results[3] };
            }

            root.put("filename", filename.data());

            pt::ptree exceptions_list;
            for (auto&& exception : data)
            {
                pt::ptree exception_node;
                exception_node.put("line", exception.line);
                exception_node.put("type", exception.type);
                exception_node.put("message", exception.message);
                exceptions_list.push_back(std::pair { "", exception_node });
            }
            root.add_child("exceptions", exceptions_list);
        }

    };
}

