//
// Created by thethongngu on 12/13/20.
//

#include "string_utils.h"

void StringUtils::remove_char(std::string &str, const char &chr) {
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == chr) str.erase(i);
    }
}

std::map<std::string, std::string> StringUtils::parse_http_req(std::string req) {
    std::map<std::string, std::string> res;
    std::vector<std::string> req_lines;

//        std::cout << "Req: \n" << req << std::endl;

    int last_pos = 0;
    for (int pos = 0; pos < req.length(); pos++) {
        if (req[pos] == '\n') {
            auto line = req.substr(last_pos, pos);
            remove_char(line, '\r');
            req_lines.push_back(line);
            last_pos = pos;
        }
    }

    // parse first line of HTTP request header
    int first_space = req_lines[0].find(' ');
    int second_space = req_lines[0].find(' ', first_space + 1);
    res["REQUEST_METHOD"] = req_lines[0].substr(0, first_space);
    res["REQUEST_URI"] = req_lines[0].substr(first_space + 1, second_space - first_space - 1);

    int question_mark = req_lines[0].find('?');
    if (question_mark == std::string::npos) {
        res["QUERY_STRING"] = "";
        res["CGI_EXEC"] = res["REQUEST_URI"].substr(1);
    } else {
        res["QUERY_STRING"] = req_lines[0].substr(question_mark + 1, second_space - question_mark - 1);
        res["CGI_EXEC"] = req_lines[0].substr(first_space + 2, question_mark - first_space - 2);
    }
    res["SERVER_PROTOCOL"] = req_lines[0].substr(second_space + 1, req_lines[0].length() - second_space - 1);


    int colon = req_lines[1].find(':');
    res["HTTP_HOST"] = req_lines[1].substr(colon + 1, req_lines[1].length() - colon);

    for (auto &e: res) {
        boost::trim(e.second);
//            std::cout << e.first << ": " << e.second << std::endl;
    }

    return std::move(res);
}

std::map<std::string, std::string> StringUtils::parse_http_query(std::string query) {
    std::vector<std::string> single;

    boost::split(single, query, boost::is_any_of("&"));

    std::map<std::string, std::string> res;
    for (auto const &q: single) {
        std::vector<std::string> pair;
        boost::split(pair, q, boost::is_any_of("="));
        boost::trim(pair[0]);
        boost::trim(pair[1]);
        res[pair[0]] = pair[1];
    }

    return res;
}

std::string StringUtils::get_column_name_html(const std::string &host, const std::string &port) {
    return R"(<th scope="col">)" + host + ":" + port + "</th>";
}

std::string StringUtils::get_column_value_html(int sess_id) {
    return R"(<td><pre id="s)" + std::to_string(sess_id) + R"(" class="mb-0"></pre></td>)";
}

std::string StringUtils::get_html_page(std::string col_name, std::string col_value) {
    boost::format html_template("<!DOCTYPE html>\n"
                                "<html lang=\"en\">\n"
                                "  <head>\n"
                                "    <meta charset=\"UTF-8\" />\n"
                                "    <title>NP Project 3 Sample Console</title>\n"
                                "    <link\n"
                                "      rel=\"stylesheet\"\n"
                                "      href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"\n"
                                "      integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"\n"
                                "      crossorigin=\"anonymous\"\n"
                                "    />\n"
                                "    <link\n"
                                "      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\n"
                                "      rel=\"stylesheet\"\n"
                                "    />\n"
                                "    <link\n"
                                "      rel=\"icon\"\n"
                                "      type=\"image/png\"\n"
                                "      href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"\n"
                                "    />\n"
                                "    <style>\n"
                                "      * {\n"
                                "        font-family: 'Source Code Pro', monospace;\n"
                                "        font-size: 1rem !important;\n"
                                "      }\n"
                                "      body {\n"
                                "        background-color: #212529;\n"
                                "      }\n"
                                "      pre {\n"
                                "        color: #cccccc;\n"
                                "      }\n"
                                "      b {\n"
                                "        color: #01b468;\n"
                                "      }\n"
                                "    </style>\n"
                                "  </head>\n"
                                "  <body>\n"
                                "    <table class=\"table table-dark table-bordered\">\n"
                                "      <thead>\n"
                                "        <tr>\n"
                                "%1%"
                                "        </tr>\n"
                                "      </thead>\n"
                                "      <tbody>\n"
                                "        <tr>\n"
                                "%2%"
                                "        </tr>\n"
                                "      </tbody>\n"
                                "    </table>\n"
                                "  </body>\n"
                                "</html>");

    html_template = html_template % col_name % col_value;
    return html_template.str();
}

std::string StringUtils::format_html(std::string res, int id, int color) {
    boost::replace_all(res, "&", "&amp;");
    boost::replace_all(res, "\"", "&quot;");
    boost::replace_all(res, "\'", "&apos;");
    boost::replace_all(res, "<", "&lt;");
    boost::replace_all(res, ">", "&gt;");
    boost::replace_all(res, "\n", "&#13;");
    boost::replace_all(res, "\r", "");

    if (color == 0) {
        res = R"(<script>document.getElementById('s)" + std::to_string(id) + R"(').innerHTML += ')" + res +
              R"(';</script>)";
    } else {
        res = R"(<script>document.getElementById('s)" + std::to_string(id) + R"(').innerHTML += '<b>)" + res +
              R"(</b>';</script>)";
    }

    return res;
}

std::string StringUtils::get_panel_html() {
    return "<!DOCTYPE html>\n"
           "<html lang=\"en\">\n"
           "  <head>\n"
           "    <title>NP Project 3 Panel</title>\n"
           "    <link\n"
           "      rel=\"stylesheet\"\n"
           "      href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"\n"
           "      integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"\n"
           "      crossorigin=\"anonymous\"\n"
           "    />\n"
           "    <link\n"
           "      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\n"
           "      rel=\"stylesheet\"\n"
           "    />\n"
           "    <link\n"
           "      rel=\"icon\"\n"
           "      type=\"image/png\"\n"
           "      href=\"https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png\"\n"
           "    />\n"
           "    <style>\n"
           "      * {\n"
           "        font-family: 'Source Code Pro', monospace;\n"
           "      }\n"
           "    </style>\n"
           "  </head>\n"
           "  <body class=\"bg-secondary pt-5\"><form action=\"console.cgi\" method=\"GET\">\n"
           "      <table class=\"table mx-auto bg-light\" style=\"width: inherit\">\n"
           "        <thead class=\"thead-dark\">\n"
           "          <tr>\n"
           "            <th scope=\"col\">#</th>\n"
           "            <th scope=\"col\">Host</th>\n"
           "            <th scope=\"col\">Port</th>\n"
           "            <th scope=\"col\">Input File</th>\n"
           "          </tr>\n"
           "        </thead>\n"
           "        <tbody>"
           "          <tr>\n"
           "            <th scope=\"row\" class=\"align-middle\">Session 1</th>\n"
           "            <td>\n"
           "              <div class=\"input-group\">\n"
           "                <select name=\"h0\" class=\"custom-select\">\n"
           "                    <option></option>\n"
           "                    <option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option>"
           "                    <option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option>"
           "                    <option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option>"
           "                    <option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option>"
           "                    <option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option>"
           "                    <option value=\"nplinux6.cs.nctu.edu.tw\">nplinux6</option>"
           "                    <option value=\"nplinux7.cs.nctu.edu.tw\">nplinux7</option>"
           "                    <option value=\"nplinux8.cs.nctu.edu.tw\">nplinux8</option>"
           "                    <option value=\"nplinux9.cs.nctu.edu.tw\">nplinux9</option>"
           "                    <option value=\"nplinux10.cs.nctu.edu.tw\">nplinux10</option>"
           "                    <option value=\"nplinux11.cs.nctu.edu.tw\">nplinux11</option>"
           "                    <option value=\"nplinux12.cs.nctu.edu.tw\">nplinux12</option>"
           "                </select>\n"
           "                <div class=\"input-group-append\">\n"
           "                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>\n"
           "                </div>\n"
           "              </div>\n"
           "            </td>\n"
           "            <td>\n"
           "              <input name=\"p0\" type=\"text\" class=\"form-control\" size=\"5\" />\n"
           "            </td>\n"
           "            <td>\n"
           "              <select name=\"f0\" class=\"custom-select\">\n"
           "                <option></option>\n"
           "                <option value=\"t1.txt\">t1.txt</option>\n"
           "                <option value=\"t2.txt\">t2.txt</option>\n"
           "                <option value=\"t3.txt\">t3.txt</option>\n"
           "                <option value=\"t4.txt\">t4.txt</option>\n"
           "                <option value=\"t5.txt\">t5.txt</option>\n"
           "                <option value=\"t6.txt\">t6.txt</option>\n"
           "                <option value=\"t7.txt\">t7.txt</option>\n"
           "                <option value=\"t8.txt\">t8.txt</option>\n"
           "                <option value=\"t9.txt\">t9.txt</option>\n"
           "                <option value=\"t10.txt\">t10.txt</option>\n"
           "              </select>\n"
           "            </td>\n"
           "          </tr>\n"
           "          <tr>\n"
           "            <th scope=\"row\" class=\"align-middle\">Session 2</th>\n"
           "            <td>\n"
           "              <div class=\"input-group\">\n"
           "                <select name=\"h1\" class=\"custom-select\">\n"
           "                    <option></option>\n"
           "                    <option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option>"
           "                    <option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option>"
           "                    <option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option>"
           "                    <option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option>"
           "                    <option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option>"
           "                    <option value=\"nplinux6.cs.nctu.edu.tw\">nplinux6</option>"
           "                    <option value=\"nplinux7.cs.nctu.edu.tw\">nplinux7</option>"
           "                    <option value=\"nplinux8.cs.nctu.edu.tw\">nplinux8</option>"
           "                    <option value=\"nplinux9.cs.nctu.edu.tw\">nplinux9</option>"
           "                    <option value=\"nplinux10.cs.nctu.edu.tw\">nplinux10</option>"
           "                    <option value=\"nplinux11.cs.nctu.edu.tw\">nplinux11</option>"
           "                    <option value=\"nplinux12.cs.nctu.edu.tw\">nplinux12</option>"
           "                </select>\n"
           "                <div class=\"input-group-append\">\n"
           "                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>\n"
           "                </div>\n"
           "              </div>\n"
           "            </td>\n"
           "            <td>\n"
           "              <input name=\"p1\" type=\"text\" class=\"form-control\" size=\"5\" />\n"
           "            </td>\n"
           "            <td>\n"
           "              <select name=\"f1\" class=\"custom-select\">\n"
           "                <option></option>\n"
           "                <option value=\"t1.txt\">t1.txt</option>\n"
           "                <option value=\"t2.txt\">t2.txt</option>\n"
           "                <option value=\"t3.txt\">t3.txt</option>\n"
           "                <option value=\"t4.txt\">t4.txt</option>\n"
           "                <option value=\"t5.txt\">t5.txt</option>\n"
           "                <option value=\"t6.txt\">t6.txt</option>\n"
           "                <option value=\"t7.txt\">t7.txt</option>\n"
           "                <option value=\"t8.txt\">t8.txt</option>\n"
           "                <option value=\"t9.txt\">t9.txt</option>\n"
           "                <option value=\"t10.txt\">t10.txt</option>\n"
           "              </select>\n"
           "            </td>\n"
           "          </tr>"
           "          <tr>\n"
           "            <th scope=\"row\" class=\"align-middle\">Session 3</th>\n"
           "            <td>\n"
           "              <div class=\"input-group\">\n"
           "                <select name=\"h2\" class=\"custom-select\">\n"
           "                    <option></option>\n"
           "                    <option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option>"
           "                    <option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option>"
           "                    <option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option>"
           "                    <option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option>"
           "                    <option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option>"
           "                    <option value=\"nplinux6.cs.nctu.edu.tw\">nplinux6</option>"
           "                    <option value=\"nplinux7.cs.nctu.edu.tw\">nplinux7</option>"
           "                    <option value=\"nplinux8.cs.nctu.edu.tw\">nplinux8</option>"
           "                    <option value=\"nplinux9.cs.nctu.edu.tw\">nplinux9</option>"
           "                    <option value=\"nplinux10.cs.nctu.edu.tw\">nplinux10</option>"
           "                    <option value=\"nplinux11.cs.nctu.edu.tw\">nplinux11</option>"
           "                    <option value=\"nplinux12.cs.nctu.edu.tw\">nplinux12</option>"
           "                </select>\n"
           "                <div class=\"input-group-append\">\n"
           "                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>\n"
           "                </div>\n"
           "              </div>\n"
           "            </td>\n"
           "            <td>\n"
           "              <input name=\"p2\" type=\"text\" class=\"form-control\" size=\"5\" />\n"
           "            </td>\n"
           "            <td>\n"
           "              <select name=\"f2\" class=\"custom-select\">\n"
           "                <option></option>\n"
           "                <option value=\"t1.txt\">t1.txt</option>\n"
           "                <option value=\"t2.txt\">t2.txt</option>\n"
           "                <option value=\"t3.txt\">t3.txt</option>\n"
           "                <option value=\"t4.txt\">t4.txt</option>\n"
           "                <option value=\"t5.txt\">t5.txt</option>\n"
           "                <option value=\"t6.txt\">t6.txt</option>\n"
           "                <option value=\"t7.txt\">t7.txt</option>\n"
           "                <option value=\"t8.txt\">t8.txt</option>\n"
           "                <option value=\"t9.txt\">t9.txt</option>\n"
           "                <option value=\"t10.txt\">t10.txt</option>\n"
           "              </select>\n"
           "            </td>\n"
           "          </tr>"
           "          <tr>\n"
           "            <th scope=\"row\" class=\"align-middle\">Session 4</th>\n"
           "            <td>\n"
           "              <div class=\"input-group\">\n"
           "                <select name=\"h3\" class=\"custom-select\">\n"
           "                    <option></option>\n"
           "                    <option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option>"
           "                    <option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option>"
           "                    <option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option>"
           "                    <option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option>"
           "                    <option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option>"
           "                    <option value=\"nplinux6.cs.nctu.edu.tw\">nplinux6</option>"
           "                    <option value=\"nplinux7.cs.nctu.edu.tw\">nplinux7</option>"
           "                    <option value=\"nplinux8.cs.nctu.edu.tw\">nplinux8</option>"
           "                    <option value=\"nplinux9.cs.nctu.edu.tw\">nplinux9</option>"
           "                    <option value=\"nplinux10.cs.nctu.edu.tw\">nplinux10</option>"
           "                    <option value=\"nplinux11.cs.nctu.edu.tw\">nplinux11</option>"
           "                    <option value=\"nplinux12.cs.nctu.edu.tw\">nplinux12</option>"
           "                </select>\n"
           "                <div class=\"input-group-append\">\n"
           "                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>\n"
           "                </div>\n"
           "              </div>\n"
           "            </td>\n"
           "            <td>\n"
           "              <input name=\"p3\" type=\"text\" class=\"form-control\" size=\"5\" />\n"
           "            </td>\n"
           "            <td>\n"
           "              <select name=\"f3\" class=\"custom-select\">\n"
           "                <option></option>\n"
           "                <option value=\"t1.txt\">t1.txt</option>\n"
           "                <option value=\"t2.txt\">t2.txt</option>\n"
           "                <option value=\"t3.txt\">t3.txt</option>\n"
           "                <option value=\"t4.txt\">t4.txt</option>\n"
           "                <option value=\"t5.txt\">t5.txt</option>\n"
           "                <option value=\"t6.txt\">t6.txt</option>\n"
           "                <option value=\"t7.txt\">t7.txt</option>\n"
           "                <option value=\"t8.txt\">t8.txt</option>\n"
           "                <option value=\"t9.txt\">t9.txt</option>\n"
           "                <option value=\"t10.txt\">t10.txt</option>\n"
           "              </select>\n"
           "            </td>\n"
           "          </tr>"
           "          <tr>\n"
           "            <th scope=\"row\" class=\"align-middle\">Session 5</th>\n"
           "            <td>\n"
           "              <div class=\"input-group\">\n"
           "                <select name=\"h4\" class=\"custom-select\">\n"
           "                    <option></option>\n"
           "                    <option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option>"
           "                    <option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option>"
           "                    <option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option>"
           "                    <option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option>"
           "                    <option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option>"
           "                    <option value=\"nplinux6.cs.nctu.edu.tw\">nplinux6</option>"
           "                    <option value=\"nplinux7.cs.nctu.edu.tw\">nplinux7</option>"
           "                    <option value=\"nplinux8.cs.nctu.edu.tw\">nplinux8</option>"
           "                    <option value=\"nplinux9.cs.nctu.edu.tw\">nplinux9</option>"
           "                    <option value=\"nplinux10.cs.nctu.edu.tw\">nplinux10</option>"
           "                    <option value=\"nplinux11.cs.nctu.edu.tw\">nplinux11</option>"
           "                    <option value=\"nplinux12.cs.nctu.edu.tw\">nplinux12</option>"
           "                </select>\n"
           "                <div class=\"input-group-append\">\n"
           "                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>\n"
           "                </div>\n"
           "              </div>\n"
           "            </td>\n"
           "            <td>\n"
           "              <input name=\"p4\" type=\"text\" class=\"form-control\" size=\"5\" />\n"
           "            </td>\n"
           "            <td>\n"
           "              <select name=\"f4\" class=\"custom-select\">\n"
           "                <option></option>\n"
           "                <option value=\"t1.txt\">t1.txt</option>\n"
           "                <option value=\"t2.txt\">t2.txt</option>\n"
           "                <option value=\"t3.txt\">t3.txt</option>\n"
           "                <option value=\"t4.txt\">t4.txt</option>\n"
           "                <option value=\"t5.txt\">t5.txt</option>\n"
           "                <option value=\"t6.txt\">t6.txt</option>\n"
           "                <option value=\"t7.txt\">t7.txt</option>\n"
           "                <option value=\"t8.txt\">t8.txt</option>\n"
           "                <option value=\"t9.txt\">t9.txt</option>\n"
           "                <option value=\"t10.txt\">t10.txt</option>\n"
           "              </select>\n"
           "            </td>\n"
           "          </tr>"
           "          <tr>"
           "            <td colspan=\"3\"></td>\n"
           "            <td>\n"
           "              <button type=\"submit\" class=\"btn btn-info btn-block\">Run</button>\n"
           "            </td>\n"
           "          </tr>\n"
           "        </tbody>\n"
           "      </table>\n"
           "    </form>\n"
           "  </body>\n"
           "</html>";
}
