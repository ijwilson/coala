#include <Rcpp.h>
#include <fstream>

using namespace Rcpp;

// Reads the 'positions:' line of ms' output and converts it into an
// NumericVector
// [[Rcpp::export]]
NumericVector parse_ms_positions(const std::string line) {
  std::stringstream stream(line);
  std::vector<double> data;

  if (line.substr(0, 11) != "positions: ") {
    stop("Failed to read positions from ms' output");
  }

  // Remove the 'positions: ' at the line's beginning
  stream.ignore(11);

  // Convert the positions into doubles
  std::copy(std::istream_iterator<double>(stream),
  std::istream_iterator<double>(),
  std::back_inserter(data));

  return(wrap(data));
}

// Reads one more files with ms output, and generates a list of segregating
// sites
// [[Rcpp::export]]
List parse_ms_output(const List file_names,
                     const NumericVector sample_size,
                     const int loci_number) {

  std::string line;
  size_t individuals = sum(sample_size);

  List seg_sites(loci_number);
  int locus = -1;

  for (int i = 0; i < file_names.size(); ++i) {
    CharacterVector file_name = as<CharacterVector>(file_names(i));
    for (int j = 0; j < file_name.size(); ++ j) {

      // Open the file
      std::ifstream output(as<std::string>(file_name(j)).c_str(),
                           std::ifstream::in);

      if (!output.is_open()) {
        stop(std::string("Cannot open file ") + file_name(0));
      }

      // Read it line by line and read the relevant parts
      while( output.good() ) {
        std::getline(output, line);
        if (line == "//") {
          ++locus;
        }

        else if (line.substr(0, 9) == "segsites:") {

          if (line.substr(0, 11) == "segsites: 0") {
            NumericMatrix ss = NumericMatrix(individuals, 0);
            ss.attr("positions") = NumericVector(0);
            seg_sites[locus] = ss;
          } else {
            std::getline(output, line);

            // Parse Seg.Sites
            NumericVector positions = parse_ms_positions(line);
            NumericMatrix ss(individuals, positions.size());
            ss.attr("positions") = positions;

            for (size_t i = 0; i < individuals; ++i) {
              std::getline(output, line);
              for (int j = 0; j < positions.size(); ++j) {
                ss(i,j) = (line[j] == '1');
              }
            }

            seg_sites[locus] = ss;
          }
        }
      }
    }
  }

  return seg_sites;
}
