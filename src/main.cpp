#include "ofMain.h"
#ifdef SHIFT
#undef SHIFT
#endif

#include <dlib/svm_threaded.h>
#include <dlib/string.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_processing.h>
#include <dlib/data_io.h>
#include <dlib/cmd_line_parser.h>

using namespace dlib;
using namespace std;

//#define GST_VIDEO
#define IMAGE
//#define PLAYER_VIDEO
class ofApp : public ofBaseApp
{
	public:
        typedef scan_fhog_pyramid<pyramid_down<6> > image_scanner_type;
	float upsample_amount = 1.0;

	array2d<dlib::rgb_pixel> images;
       	object_detector<image_scanner_type> detector;
	std::vector<rectangle> rects;

	#ifdef GST_VIDEO
	ofGstVideoUtils cam;
        int w,h;
	#endif

	#ifdef PLAYER_VIDEO 
	ofVideoPlayer video;
	#endif

	#ifdef IMAGE
	ofImage img;
	#endif
	
        dlib::array2d<dlib::rgb_pixel> toDLib(const ofPixels px)
        {
            dlib::array2d<dlib::rgb_pixel> out;
            int width = px.getWidth();
            int height = px.getHeight();
            int ch = px.getNumChannels();

            out.set_size( height, width );
            const unsigned char* data = px.getData();
            for ( unsigned n = 0; n < height;n++ )
            {
                const unsigned char* v =  &data[n * width *  ch];
                for ( unsigned m = 0; m < width;m++ )
                {
                    if ( ch==1 )
                    {
                        unsigned char p = v[m];
                        dlib::assign_pixel( out[n][m], p );
                    }
                    else{
                        dlib::rgb_pixel p;
                        p.red = v[m*3];
                        p.green = v[m*3+1];
                        p.blue = v[m*3+2];
                        dlib::assign_pixel( out[n][m], p );
                    }
                }
            }
            return out;
        }

	void setup()
	{
		ifstream fin(ofToDataPath("inria_person.svm"), ios::binary);
	        deserialize(detector, fin);

		#ifdef GST_VIDEO
			string ip = "172.16.200.235";
			int port=5000;
			w = 480;
			h = 320;
			cam.setPipeline("tcpclientsrc host="+ip+" port="+ofToString(port)+" ! gdpdepay ! rtph264depay ! avdec_h264 ! videoconvert", OF_PIXELS_RGB, true, w, h);
			cam.startPipeline();
			cam.play();
		#endif

		#ifdef PLAYER_VIDEO
			video.load("movie.mp4");
		        video.setLoopState(OF_LOOP_NORMAL);
		        video.play();
		#endif

		#ifdef IMAGE
			img.load("test1.jpg");
			images = toDLib(img.getPixels());
        	    	for (unsigned long i = 0; i < upsample_amount; i++)
            		{
	        		pyramid_up(images);
        	    	}
			rects = detector(images);
		        cout << "Number of detections: "<< rects.size() << endl;
		#endif
	}

	void update()
	{
	        ofSetWindowTitle(ofToString(ofGetFrameRate()));
		#ifdef GST_VIDEO
		cam.update();
                if(cam.isFrameNew())
                {
			images = toDLib(cam.getPixels());
	            	for (unsigned long i = 0; i < upsample_amount; i++)
        	    	{
              			pyramid_up(images);
            		}
			rects = detector(images);
	        	cout << "Number of detections: "<< rects.size() << endl;
                }
		#endif 

		#ifdef PLAYER_VIDEO
		video.update();
		if(video.isFrameNew())
		{
			images = toDLib(video.getPixels());
                        for (unsigned long i = 0; i < upsample_amount; i++)
                        {
                                pyramid_up(images);
                        }
                        rects = detector(images);
			if(rects.size()>0)
	                        ofLog()<< "Number of detections: "<< rects.size();
		}
		#endif
	}

	void draw()
	{
		ofSetColor(ofColor::white);
		#ifdef GST_VIDEO
			ofImage i = cam.getPixels();
			i.draw(0,0);
		#endif

		#ifdef PLAYER_VIDEO
			video.draw(0,0);
		#endif

		#ifdef IMAGE
			img.draw(0,0);
		#endif

		ofNoFill();
		ofSetColor(ofColor::red);

		for(int i = 0; i < rects.size(); i++)
			ofDrawRectangle(rects[i].left()/2,rects[i].top()/2,rects[i].width()/2,rects[i].height()/2);
	}
};

int main(int argc, char *argv[])
{
	ofSetupOpenGL(1024,768, OF_WINDOW);
	ofRunApp( new ofApp() );
}
