/*
 * Autores: Aron Collados (626558)
 *          Cristian Roman (646564)
 */

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <sstream>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>

using namespace cv;
using namespace std;

bool rgbH=false;
typedef Mat (*Effect)(Mat);

/*
 * Devuelve el histograma de grises de una imagen
 */
Mat create_histogram_image(Mat bgrMap)
{

  int hist_size = 256;
  float range[]={0,256};
  const float* ranges[] = { range };
  float max_value = 0.0, min_value = 0.0;
  float w_scale = 0.0;
  if(bgrMap.type()==16){ // Solo aplica escala de grises si no se ha aplicado antes
  	cvtColor( bgrMap, bgrMap, CV_BGR2GRAY );
  }

  int bin_w = cvRound( (double) bgrMap.cols/hist_size );
  Mat hist,histImage( bgrMap.rows, bgrMap.cols, CV_8UC3, Scalar( 0,0,0) );

  calcHist( &bgrMap, 1, 0, Mat(), hist, 1, &hist_size, ranges, true, false );
  normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

  for(int i = 0; i < hist_size; i++ ){
        line( histImage, Point( bin_w*(i-1), histImage.rows - cvRound(hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), histImage.rows - cvRound(hist.at<float>(i)) ),
                       Scalar( 255, 255, 255), 2, 8, 0  );
  }
  return histImage;
}

/*
 * Devuelve el histograma con los valores RGB de una imagen
 * Basado en http://docs.opencv.org/doc/tutorials/imgproc/histograms/histogram_calculation/histogram_calculation.html
 */
Mat create_histogram_image_rgb(Mat bgrMap)
{
  Mat histImage( bgrMap.rows, bgrMap.cols, CV_8UC3, Scalar( 0,0,0) );
  if(bgrMap.type()==0){ // No muestra nada si esta en escala de grises 
  	return histImage;
  }
  int hist_size = 256;
  float range[]={0,256};
  const float* ranges[] = { range };
  float max_value = 0.0, min_value = 0.0;
  float w_scale = 0.0;
  vector<Mat> bgr;
  split( bgrMap, bgr );  //Separa una imagen en capas


  int bin_w = cvRound( (double) bgrMap.cols/hist_size );

  Mat r,g,b;
  calcHist( &bgr[0], 1, 0, Mat(), b, 1, &hist_size, ranges, true, false );
  normalize(b, b, 0, bgrMap.rows, NORM_MINMAX, -1, Mat() );
  
  calcHist( &bgr[1], 1, 0, Mat(), g, 1, &hist_size, ranges, true, false );
  normalize(g, g, 0, bgrMap.rows, NORM_MINMAX, -1, Mat() );
  
  calcHist( &bgr[2], 1, 0, Mat(), r, 1, &hist_size, ranges, true, false );
  normalize(r, r, 0, bgrMap.rows, NORM_MINMAX, -1, Mat() );
  
  for(int i = 0; i < hist_size; i++ ){
        line( histImage, Point( bin_w*(i-1), histImage.rows - cvRound(r.at<float>(i-1)) ) ,
                       Point( bin_w*(i), histImage.rows - cvRound(r.at<float>(i)) ),
                       Scalar( 255, 0, 0), 2, 8, 0  );
	line( histImage, Point( bin_w*(i-1), histImage.rows - cvRound(g.at<float>(i-1)) ) ,
                       Point( bin_w*(i), histImage.rows - cvRound(g.at<float>(i)) ),
                       Scalar( 0,255, 0), 2, 8, 0  );
	line( histImage, Point( bin_w*(i-1), histImage.rows - cvRound(b.at<float>(i-1)) ) ,
                       Point( bin_w*(i), histImage.rows - cvRound(b.at<float>(i)) ),
                       Scalar( 0, 0, 255), 2, 8, 0  );
  }
  return histImage;
}
/*
 * Filtro de ruido
 */
Mat removeNoise(Mat src) {
    Mat dst = src.clone();
    int v[3][3] = {{1, 2, 1},{2, 4, 2},{1, 2, 1} };
    //int v[5][5]={{1,4,6,4,1},{4,16,24,16,4},{6,24,36,24,6},{4,16,24,16,4},{1,4,6,4,1}};
    int m = 0;
    size_t r = sizeof (*v) / sizeof (*v[0]), c = r;
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            m += v[i][j];
        }
    }

    int cr = r / 2, cc = c / 2;

    for (int i = cr; i < src.rows - cr; i++) {
        for (int j = cc; j < src.cols - cc; j++) {
            int tr = 0, tg = 0, tb = 0;
            for (int a = 0; a < r; a++) {
                for (int b = 0; b < c; b++) {
                    int coe = v[a][b];
                    tr += src.at<Vec3b>(i + a - cr, j + b - cc)[0] * coe;
                    tg += src.at<Vec3b>(i + a - cr, j + b - cc)[1] * coe;
                    tb += src.at<Vec3b>(i + a - cr, j + b - cc)[2] * coe;
                }
            }
            dst.at<Vec3b>(i, j)[0] = tr / m;
            dst.at<Vec3b>(i, j)[1] = tg / m;
            dst.at<Vec3b>(i, j)[2] = tb / m;
        }
    }
    return dst;
}

/*
 * Grises
 */
Mat Grises(Mat bgrMap){
  Mat dst;
  cvtColor( bgrMap, dst, CV_RGB2GRAY );
  return dst;
}

/*
 * Otsu effect
 */
Mat Otsu(Mat src){
  Mat dst;
  threshold(src, dst, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
  return dst;
}

/*
 * Adatative effect
 */
Mat adaptative(Mat src){
   Mat dst;
   adaptiveThreshold(src, dst,255,CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV,75,10);
   return dst;
}

/*
 * Main principal
 */
int main(int argc, char *argv[]) {
    char key = 0;
    int numSnapshot = 0;
    std::string snapshotFilename = "0";
    std::cout << "Pulsa 'espacio'para hacer una captura" << std::endl;
    std::cout << "Pulsa 'escape' para salir" << std::endl;

    Mat bgrMap,captura,NuevaImagen;

    //Inicializa las ventanas
    namedWindow("BGR image",  WINDOW_KEEPRATIO);
    namedWindow("Nueva Imagen",  WINDOW_KEEPRATIO);
    namedWindow("Histograma original", WINDOW_KEEPRATIO);
    namedWindow("Histograma destino", WINDOW_KEEPRATIO);
    Effect binaryEffect=*Otsu,change=*adaptative;

    std::string arg = argc>1?argv[1]:"0";   

    captura = imread(arg, CV_LOAD_IMAGE_COLOR); //Carga la imagen recibida por parametro
    VideoCapture TheVideoCapturer(arg);
    if (captura.empty()&&!TheVideoCapturer.isOpened()) 
        {
		TheVideoCapturer.open(atoi(arg.c_str())); //Abre la videocamara
	}
           
    if (!TheVideoCapturer.isOpened()&&captura.empty()) {
 	std::cerr << "Could not open file " << arg << std::endl;
        return -1;
    }
    if (!captura.empty()){
    	bgrMap=captura;
    }
    while (key != 27 && (!captura.empty() || TheVideoCapturer.grab())) {
        if(captura.empty()){ //Se esta usando una camara
        	TheVideoCapturer >> bgrMap;
        }  
	NuevaImagen=binaryEffect(Grises(bgrMap));
        imshow("BGR image", bgrMap); //Muestra por pantalla
        imshow("Nueva Imagen", NuevaImagen);
	Mat histogram_O,histogram_N;

	if(rgbH){
		histogram_O=create_histogram_image_rgb(bgrMap);
		histogram_N=create_histogram_image_rgb(NuevaImagen);
	}else{
		histogram_O=create_histogram_image(bgrMap);
		histogram_N=create_histogram_image(NuevaImagen);
	}
  	imshow( "Histograma original", histogram_O );
  	imshow( "Histograma destino", histogram_N );

        switch (key) { //Códigos aqui http://www.asciitable.com/

            case 32: //space
                std::cout << "Imagen guardada "<< snapshotFilename << ".png" << std::endl;
                imwrite(snapshotFilename + "_N.png", NuevaImagen);
                imwrite(snapshotFilename + "_O.png", bgrMap);
                imwrite(snapshotFilename + "_HN.png", histogram_N);
                imwrite(snapshotFilename + "_HO.png", histogram_O);
                numSnapshot++;
                snapshotFilename = static_cast<std::ostringstream*> (&(std::ostringstream() << numSnapshot))->str();
                break;
           case 104://h
                if (rgbH) {
                    std::cout << "Histograma modo grises" << std::endl;

                } else {
                    std::cout << "Histograma modo rgb" << std::endl;
                }
                rgbH = !rgbH;
                break;
	   case 109://m
		Effect aux=binaryEffect;
		binaryEffect=change;
		change=aux;
                std::cout << "Modo binario cambiado" << std::endl;
                break;
        }


        key = waitKey(20);
    }
}