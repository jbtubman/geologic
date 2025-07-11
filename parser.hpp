#pragma once

#include <print>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include "cell_types.hpp"
#include "column.hpp"
#include "jt_concepts.hpp"
#include "parse_utils.hpp"
#include "utility.hpp"

namespace jt {
using std::println;
using std::regex;
using std::string;
using std::string_view;
using std::vector;
namespace ranges = std::ranges;
namespace views = std::views;
using namespace std::string_literals;

inline static auto infinite_ints_vw() {
    std::size_t zero{0};
    return views::iota(zero);
}

class parser {
   public:
    class header_field {
       public:
        string name{};
        e_cell_data_type data_type{e_cell_data_type::undetermined};

        bool operator==(const header_field& other) const {
            return data_type == other.data_type && name == other.name;
        }
    };

    using header_fields = vector<header_field>;

    /// A data field represents a single field read from a data row of a CSV
    /// file. It does not have the actual parsed value (int, geo-coordinate,
    /// whatever.)
    class data_field {
       public:
        string text{""};
        e_cell_data_type data_type{e_cell_data_type::undetermined};

        bool operator==(const data_field& other) const {
            return data_type == other.data_type && text == other.text;
        }
    };

    using data_fields = vector<data_field>;
    using all_data_fields = vector<data_fields>;

    class header_and_data {
       public:
        header_fields header_fields_{};
        all_data_fields data_fields_vec_{};

        header_and_data() {}

        header_and_data(const header_fields& hfs) : header_fields_{hfs} {}

        header_and_data(header_fields&& hfs) : header_fields_{std::move(hfs)} {}

        template <class HeaderFields, class AllDataFields>
        header_and_data(HeaderFields&& hfs, AllDataFields&& adfs)
            : header_fields_{std::forward<HeaderFields>(hfs)},
              data_fields_vec_{std::forward<AllDataFields>(adfs)} {}

        const all_data_fields& get_data_fields() const {
            return data_fields_vec_;
        }
        all_data_fields& get_data_fields() { return data_fields_vec_; }
    };

    /// @brief determine the cell data types for all the columns.
    /// @param all_df vector of vector of data_field objects.
    /// @return vector of e_cell_data_type values.
    static vector<e_cell_data_type> get_data_types_for_all_columns(
        const parser::all_data_fields& all_df) {
        const auto row_count = all_df.size();
        const auto column_count = row_count > 0 ? all_df[0].size() : 0;
        vector<e_cell_data_type> result{};

        // Slice through the rows a column at a time.
        for (size_t column_index = 0; column_index < column_count;
             ++column_index) {
            e_cell_data_type ecdt = e_cell_data_type::undetermined;
            for (size_t row_index = 0; row_index < row_count; ++row_index) {
                const parser::data_fields the_row = all_df[row_index];
                const parser::data_field the_data_field = the_row[column_index];
                ecdt = ecdt || the_data_field.data_type;
            }
            result.push_back(ecdt);
        }

        return result;
    }

    static header_fields parse_header(const string& header) {
        using std::operator""sv;

        header_fields result;

        std::string_view header_sv{header};

        auto split_header =
            header_sv | views::split(","sv) | ranges::to<vector<string>>();

        ranges::transform(split_header, std::back_inserter(result),
                          [](auto header_text) {
                              return header_field{
                                  header_text, e_cell_data_type::undetermined};
                          });

        return result;
    }

    static data_fields parse_data_row(const string& data_row) {
        data_fields result;

        auto split_row =
            data_row | views::split(","s) | ranges::to<vector<string>>();

        auto fixed_split_row = fix_quoted_fields(split_row);

        auto foo = ranges::transform(
            fixed_split_row, std::back_inserter(result),
            [](auto data_row_text) {
                return data_field{
                    data_row_text,
                    determine_data_field_e_cell_data_type(data_row_text)};
            });
        return result;
    }

    /**
     * @brief Turns a vector of strings into a vector of e_cell_data_type
     * values.
     * TODO: is this used?
     */
    static vector<e_cell_data_type> row_value_types(
        const vector<string>& split_fields) {
        auto result = split_fields | views::transform([](const string& s) {
                          return determine_data_field_e_cell_data_type(s);
                      }) |
                      ranges::to<vector<e_cell_data_type>>();
        return result;
    }
};

// Free functions for converting the above classes to strings for debugging
// purposes.

inline string str(const parser::header_field& hf) {
    return "header_field{ name: \""s + hf.name + "\", data_type: "s +
           str(hf.data_type) + " }"s;
}

inline string str(const parser::header_fields& hfs) {
    string result("header_fields{ \n\t");
    for (const parser::header_field& hf : hfs) {
        result.append(str(hf) + ", "s);
    }
    result = string(result.begin(), result.begin() + result.find_last_of(","));
    result.append(" }\n");
    return result;
}

inline string str(const parser::data_field& df) {
    return string{"data_field{ text: \""s + df.text + "\", data_type: "s +
                  str(df.data_type) + " }"};
}

inline string str(const parser::data_fields& dfs) {
    string result{"[ "s};
    for (const auto& df : dfs) {
        result.append(str(df) + ", "s);
    }
    result = string(result.begin(), result.begin() + result.find_last_of(","));
    result.append(" ]");
    return result;
}

inline string str(const parser::all_data_fields& adfs) {
    string result("{ \n\t");
    for (const parser::data_fields& dfs : adfs) {
        result.append(str(dfs) + ", \n"s);
    }
    result = string(result.begin(), result.begin() + result.find_last_of(","));
    result.append(" }");
    return result;
}

static inline parser::data_fields get_data_fields_for_one_column(
    const parser::all_data_fields& all_df, size_t column_index) {
    return ranges::fold_left(all_df, parser::data_fields{},
                             [column_index](auto&& acc, auto&& v) {
                                 return shove_back(acc, v[column_index]);
                             });
}

static inline e_cell_data_type get_data_type_for_column(
    const parser::data_fields& dfs_for_one_column) {
    return ranges::fold_left(
        dfs_for_one_column, e_cell_data_type::undetermined,
        [](e_cell_data_type acc, const parser::data_field& df) {
            return acc || df.data_type;
        });
}

template <typename VecString>
parser::header_and_data _parse_lines(VecString&& input_lines) {
    auto in_lines = std::move<VecString>(input_lines);
    auto len = in_lines.size();
    auto first = in_lines.begin();
    auto last = in_lines.end();
    if (first == last) {
        return parser::header_and_data();
    }

    auto second = ++first;
    if (second == last) {
        println(stderr, "No data rows");
        return parser::header_and_data(parser::parse_header(in_lines[0]));
    } else {
        println(stderr, "{} data rows", last - second);
    }

    parser::header_and_data result(parser::parse_header(in_lines[0]));
    auto data_range = ranges::subrange(second, last);

    for (auto current = second; current != last; ++current) {
        println(stderr, "data row: \"{}\"", *current);
        const string& s = *current;
        parser::data_fields dfs = parser::parse_data_row(s);
        result.data_fields_vec_.push_back(dfs);
        println(stderr, "result.data_fields_vec_.size(): {}",
                result.data_fields_vec_.size());
    }

    const vector<e_cell_data_type> cell_data_types_vec =
        parser::get_data_types_for_all_columns(result.data_fields_vec_);

    // Add the cell data info to the headers.
    parser::header_fields& hfs = result.header_fields_;
    parser::header_fields hfs_result{};

    auto zipped = ranges::zip_view(hfs, cell_data_types_vec);

    for( std::pair<parser::header_field, e_cell_data_type> hf_cdt : zipped) {
        hfs_result.emplace_back(hf_cdt.first.name, hf_cdt.second);
        // hf_cdt.first.data_type = hf_cdt.second;
        // hfs_result.push_back(hf_cdt);
    }
    result.header_fields_ = hfs_result;

    return result;
}

//static inline
parser::header_and_data parse_lines(const vector<string>& input_lines);
// {
//     return _parse_lines<const vector<string>&>(input_lines);
// }

// static inline
parser::header_and_data parse_lines(vector<string>&& input_lines);
// {
//     return _parse_lines(input_lines);
// }

}  // namespace jt
