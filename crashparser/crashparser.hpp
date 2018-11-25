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

    class crashparser
    {
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

            pt::ptree exceptions_list;

            for (
                    std::sregex_iterator it { in.begin(), in.end(), exception_pattern }, it_end;
                    it != it_end;
                    ++it
                    )
            {
                auto& match_results = *it;

                pt::ptree exception_node;
                exception_node.put("line", match_results[1]);
                exception_node.put("type", match_results[2]);
                exception_node.put("message", match_results[3]);
                exceptions_list.push_back(std::pair { "", exception_node });
            }

            root.put("filename", filename.data());
            root.add_child("exceptions", exceptions_list);
        }

    };
}

