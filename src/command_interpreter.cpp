#include "command_interpreter.hpp"

#include <expected>
#include <iostream>
#include <print>
#include <regex>
#include <sstream>
#include <string>

#include "cell_types.hpp"
#include "query.hpp"
#include "table.hpp"
#include "utility.hpp"

namespace jt {
using std::cerr;
using std::endl;
using std::expected;
using std::println;
using std::regex;
using std::regex_match;
using std::regex_search;
using std::smatch;
using std::string;
using std::unexpected;
using ecdt = e_cell_data_type;

enum class query_error { bad_format };

string row_to_string(const row& rw) {
    std::ostringstream sout{};
    bool first_column = true;
    ranges::for_each(rw, [&first_column, &sout](const data_cell& dc) {
        if (!first_column) {
            sout << ",";
        }
        first_column = false;
        if (dc.value) {
            sout << cell_value_types_value_as_string(*(dc.value));
        } else {
            sout << "";
        }
    });

    return sout.str();
}

void command_line::do_query(table& t, const string& query_line) {
    // Most of this stuff should be refactored elsewhere.
    const string query_prefix_s{R"-(^\s*query\s*)-"};

    // m[1] is the column name.
    // m[2] is the query value string.
    regex text_query_rx{R"-(^\s*query\s*\("([^"]+)"\s+"([^"]+)"\s*\)\s*$)-",
                        regex::icase};

    // m[1] is column name.
    // m[2] is integer value string.
    regex integer_query_rx{
        R"-(/^\s*query\s*\(\s*"([^"]+)"\s+(-?\d+)\s*\)\s*$/mug)-"};

    // m[1] is column name.
    // m[2] is float value string.
    regex floating_query_rx{
        R"-(^\s*query\s*\(\s*"([^"]+)"\s+(-?\d+\.\d+)\s*\)\s*$)-",
        regex::icase};

    // Used regex101.com to work this out.
    // https://regex101.com/r/5AgL7v/1
    // This one matches queries like:
    // query ("Filename" "Iceland.png")
    //  query ("Type"  "png")
    // query("Image X" 600)
    regex simple_query_rx{
        // R"(^\s*query\s*(\(\s*(".+")\s+((".+")|(.+))\s*\))\s*$)",
        R"-(^\s*query\s*\(\s*"(.*)"\s+(("(.*)")|(.*))\s*\)\s*$)-",
        // R"(^\s*query\s*.*$)",
        regex::icase};

    smatch m;
    // m[1] is the column name.
    // m[4] is the query value.

    regex_search(query_line, m, simple_query_rx);
    if (m.empty()) {
        println(stderr, "could not parse query \"{}\"", query_line);
        return;
        // Will need to change this when handling more complex queries.
    }

    const string column_name = m[1].str();
    // m[4] for string values; m[5] for integers.
    const string query_value_s = m[4].str().empty() ? m[5].str() : m[4].str();
    table::rows results;
    // Determine the column's value type and dispatch the query
    // appropriately.
    ecdt col_type = t.column_type(column_name);
    if (col_type == ecdt::text) {
        results = string_match(t, column_name, query_value_s);
    } else if (col_type == ecdt::boolean) {
        const auto query_value = s_to_boolean(query_value_s);
        if (query_value) {
            const bool query_b = *query_value;
            results = boolean_match(t, column_name, query_b);
        }
    } else if (col_type == ecdt::floating) {
        const auto query_value = s_to_floating(query_value_s);
        if (query_value) {
            const float query_f = *query_value;
            results = floating_match(t, column_name, query_f);
        }
    } else if (col_type == ecdt::geo_coordinate) {
        const auto query_value = s_to_geo_coordinate(query_value_s);
        if (query_value) {
            const coordinate coord = *query_value;
            results = geo_query_match(t, column_name, coord);
        }
    } else if (col_type == ecdt::integer) {
        const auto query_value = std::stoi(query_value_s);
        results = integer_match(t, column_name, query_value);
    } else if (col_type == ecdt::tags) {
        // TODO
    } else {
        auto expected_idx = t.index_for_column_name(column_name);
        if (!expected_idx) {
            println(stderr, "Column \"{}\" is not in this file.", column_name);
            println(stderr,
                    "Use the \"describe\" command to see the column names and "
                    "types.");
        }
    }

    // Print out the column names.
    bool first_field = true;
    string column_names_output{};
    ranges::for_each(t.header_fields_, [&first_field, &column_names_output](
                                           const parser::header_field& hf) {
        if (!first_field) {
            column_names_output.append(",");
        }
        first_field = false;
        column_names_output.append(hf.name);
    });
    println("{}", column_names_output);

    // print the rows.

    using zzz = decltype(results[0]);
    bool first_row = true;
    ranges::for_each(results, [](const row& rw) {
        string row_str = row_to_string(rw);
        println("{}", row_str);
    });
    println("{} rows found", results.size());
}
}  // namespace jt
