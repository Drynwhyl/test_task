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

    /**
     * Reads log text from input_stream and inserts parsed data using inserter iterator
     */
    template<typename IteratorT>
    void parse(std::istream& input_stream, IteratorT inserter)
    {
        static_assert(
                std::is_same_v<
                        std::output_iterator_tag,
                        typename std::iterator_traits<IteratorT>::iterator_category
                >,
                "IteratorT must meet the requirements of OutputIterator"
        );

        static std::regex const exception_pattern {
                R"REGEX(File ".+", line (\d+), in .+\s+(?:.*\s+)?(\w+): ([^\n]+)(?:\n|$))REGEX"
        };

        std::string in { std::istreambuf_iterator { input_stream }, {} };

        for (
                std::sregex_iterator it { in.begin(), in.end(), exception_pattern }, it_end;
                it != it_end;
                ++it
                )
        {
            auto results = *it;
            exception_data data { results[1], results[2], results[3] };
            *inserter = data;
            ++inserter;
        }
    }

    /**
     * Writes parsed exception data to output_stream output_stream using json format
     */
    template<typename IteratorT>
    void write_json(std::ostream& output_stream, std::string_view filename, IteratorT it, IteratorT it_end)
    {
        static_assert(
                std::is_base_of_v<
                        std::input_iterator_tag,
                        typename std::iterator_traits<IteratorT>::iterator_category
                >,
                "IteratorT must meet the requirements of InputIterator"
        );

        static_assert(
                std::is_same_v<
                        exception_data,
                        typename std::iterator_traits<IteratorT>::value_type
                >,
                "IteratorT must have value_type to be and exception_data"
        );

        pt::ptree root;

        root.put("filename", filename.data());

        pt::ptree exceptions_node;
        for (; it != it_end; ++it)
        {
            pt::ptree exception_data;
            exception_data.put("line", it->line);
            exception_data.put("type", it->type);
            exception_data.put("message", it->message);
            exceptions_node.push_back(std::pair { "", exception_data });
        }
        root.add_child("exceptions", exceptions_node);

        pt::write_json(output_stream, root);
    }

    /**
     * Opens file with given file name, parses it's content
     * and writes parsed data to output stream
     */
    inline void parse_file(std::string_view filename, std::ostream& out)
    {
        std::ifstream file { filename.data() };

        if (!file.good()) throw std::runtime_error { strerror(errno) };

        std::list<exception_data> data;
        parse(file, std::back_inserter(data));
        write_json(out, filename, data.begin(), data.end());
    }
};

