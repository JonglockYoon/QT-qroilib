
/*!
  \file        test_voronoi.cpp
  \author      Arnaud Ramey <arnaud.a.ramey@gmail.com>
                -- Robotics Lab, University Carlos III of Madrid
  \date        2013/9/19
  
________________________________________________________________________________

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
________________________________________________________________________________

Some tests for VoronoiThinner

 */

#include <opencv2/opencv.hpp>

#include "voronoithinner.h"

//int codec = CV_FOURCC('M', 'P', '4', '2');
//int codec = CV_FOURCC('M', 'J', 'P', 'G');
// from http://opencv.willowgarage.com/wiki/VideoCodecs
//int codec = CV_FOURCC('I', '4', '2', '0'); // uncompressed
//int codec = 0;

class VoronoiIterator {
public:
  VoronoiIterator() {}

  void init(const cv::Mat1b & query,
            std::string implementation_name_,
            bool crop_img_before_) {
    _implementation_name = implementation_name_;
    _crop_img_before = crop_img_before_;
    _first_img = query.clone();
    VoronoiThinner::copy_bounding_box_plusone(query, _first_img, true);
    _curr_skel = _first_img.clone();
    // printf("first_img:%s\n", image_utils::infosImage(_first_img).c_str());
    _curr_iter = 1;
    _nframes = 0;
  }

  inline cv::Mat1b first_img() {
    return _first_img.clone();
  }

  cv::Mat1b current_skel() const {
    return _thinner.get_skeleton().clone();
  }

  inline cv::Mat1b contour_brighter(const cv::Mat1b & img) {
    _contour_viz.from_image_C4(img);
    cv::Mat1b ans;
    _contour_viz.copyTo(ans);
    return ans;
  }

  inline cv::Mat3b contour_color(const cv::Mat1b & img) {
    _contour_viz.from_image_C4(img);
    return _contour_viz.illus().clone();
  }

  //! \return true if success
  bool iter() {
    ++_nframes;
    bool reuse = (_implementation_name != IMPL_MORPH); // cant reuse with morph implementation
    bool success = false;
    if (reuse)
      success = _thinner.thin(_curr_skel, _implementation_name, false, 1); // already cropped
    else
      success = _thinner.thin(_first_img, _implementation_name, false, _nframes); // already cropped
    _thinner.get_skeleton().copyTo(_curr_skel);
    return success;
  }

  inline bool has_converged() const { return _thinner.has_converged(); }

  inline int cols() const { return _first_img.cols; }
  inline int rows() const { return _first_img.rows; }

  //protected:
  std::string _implementation_name;
  bool _crop_img_before;
  int _nframes;
  int _curr_iter;
  cv::Mat1b _first_img;
  cv::Mat1b _curr_skel;
  VoronoiThinner _thinner;
  ImageContour _contour_viz;
}; // end class VoronoiIterator

#if 0

// from drawing_utils.h
template<class T>
void paste_images_gallery(const std::vector<cv::Mat_<T> > & in,
                cv::Mat_<T> & out,
                int gallerycols, T background_color,
                bool draw_borders = false, cv::Scalar border_color = CV_RGB(0,0,0)) {
  if (in.size() == 0) {
    out.create(0, 0);
    return;
  }
  int cols1 = in[0].cols, rows1 = in[0].rows, nimgs = in.size();
  // prepare output
  int galleryrows = std::ceil(1. * nimgs / gallerycols);
  out.create(rows1 * galleryrows, cols1 * gallerycols);
  // printf("nimgs:%i, gallerycols:%i, galleryrows:%i\n", nimgs, gallerycols, galleryrows);
  out.setTo(background_color);
  // paste images
  for (int img_idx = 0; img_idx < nimgs; ++img_idx) {
    int galleryx = img_idx % gallerycols, galleryy = img_idx / gallerycols;
    cv::Rect roi(galleryx * cols1, galleryy * rows1, cols1, rows1);
    // printf("### out:%ix%i, roi %i:'%s'\n", out.cols, out.rows, img_idx, geometry_utils::print_rect(roi).c_str());
    if (cols1 != in[img_idx].cols || rows1 != in[img_idx].rows) {
      printf("Image %i of size (%ix%i), different from (%ix%i), skipping it.\n",
             img_idx, in[img_idx].cols, in[img_idx].rows, cols1, rows1);
      cv::line(out, roi.tl(), roi.br(), border_color, 2);
      cv::line(out, roi.br(), roi.tl(), border_color, 2);
    }
    else
      in[img_idx].copyTo( out(roi) );
    if (draw_borders)
      cv::rectangle(out, roi, border_color, 1);
  } // end loop img_idx
} // end paste_images_gallery<_T>

////////////////////////////////////////////////////////////////////////////////


int CLI_thin() {
  // detect order
  //int order = -1;
 

  VoronoiThinner thinner;
  // load files
  std::vector<cv::Mat1b> files;
  cv::Mat1b file = cv::imread("blu13_1_Proc1.bmp", CV_LOAD_IMAGE_GRAYSCALE);
  files.push_back(file);

  // process
    for (unsigned int file_idx = 0; file_idx < files.size(); ++file_idx) 
	{
      //Timer timer;
		bool ok = thinner.thin(files[file_idx], IMPL_GUO_HALL_FAST, true);
      if (!ok) {
		  printf("Failed thinning with implementation '%s'\n", IMPL_GUO_HALL_FAST);
        continue;
      }
      //timer.printTime("implementation_name");
      // write file
      std::ostringstream out; out << "out_" << file_idx << ".png";
      cv::imwrite(out.str(), thinner.get_skeleton());
      //printf("Written file '%s'\n", out.str().c_str());
      // show res
      //cv::imshow("query", files[file_idx]);
	  cv::imshow(IMPL_GUO_HALL_FAST, thinner.get_skeleton());
      cv::waitKey(0);
    } // end loop file_idx

  return 0;
} // end CLI()

#endif
