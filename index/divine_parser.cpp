#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <boost/foreach.hpp>


using namespace std;

void process(map<string, string>& json, const string& wholeLine) {
    if (wholeLine != "") {
        size_t i;
        for (i = 0; i < wholeLine.size(); ++i) {
            if (wholeLine[i] == ',') {
                break;
            }
        }

        string prefix = wholeLine.substr(0, i);
        string suffix = wholeLine.substr(i + 1 , wholeLine.size() - 1);

        // crop quotes
        suffix = suffix.substr(1, suffix.size() - 2);

        json[prefix] = suffix;
    }
}

int main() {
    ifstream fin("input.txt");
    ofstream fout("output.txt");

    string headerLine;

    // get header (no info there)
    getline(fin, headerLine);

    map <string, string> json;

    string wholeLine = "";
    string dataLine;

    // read file line by line until end of file
    while (getline(fin, dataLine)) {

        if (dataLine.find(',') != string::npos) {
            // process previous whole line

            process(json, wholeLine);
            // start new whole line
            wholeLine = dataLine;
        } else {

            // proceed with current whole line
            wholeLine += dataLine;
        }
    }

    // the last one!
    process(json, wholeLine);

    cerr << json.size() << endl;

    // print everything!

    fout << "{" << endl;

    map<string, string>::iterator it;
    size_t count = 0;

    for (map<string, string>::iterator it = json.begin(); it != json.end(); ++it ) {

        fout << "\"" << it->first << "\" : \"" << it->second << "\"";

        if (count != json.size() - 1) {
            fout << ", ";
        }

        ++count;
        fout << endl;
    }

    fout << "}" << endl;


    // get vectors

    for (map<string, string>::iterator it = json.begin(); it != json.end(); ++it ) {
        cerr << it->first << endl;


        vector<double> myCurrentVector;

        string vectorRepresentation = it->second;
        // crop braces
        vectorRepresentation = vectorRepresentation.substr(1, vectorRepresentation.size() - 2);

        typedef std::vector<std::string> Tokens;
        Tokens tokens;
        boost::split( tokens, vectorRepresentation, boost::is_any_of(" ") );

        BOOST_FOREACH( const std::string& i, tokens ) {
            if (i != "") {
                // std::cout << << std::endl;
                double current = std::atof(i.c_str());
                myCurrentVector.push_back(current);
            }
                    }

        cerr << endl;
        cerr << "vector size is: " << myCurrentVector.size() << endl;

        for (size_t i = 0 ; i < myCurrentVector.size(); ++i) {
            cerr << myCurrentVector[i] << " ";
        }

        cerr << endl;
    }
}
