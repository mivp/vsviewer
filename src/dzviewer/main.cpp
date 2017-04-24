/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
** Copyright (c) 2015, Monash University
** All rights reserved.
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**
**       * Redistributions of source code must retain the above copyright notice,
**          this list of conditions and the following disclaimer.
**       * Redistributions in binary form must reproduce the above copyright
**         notice, this list of conditions and the following disclaimer in the
**         documentation and/or other materials provided with the distribution.
**       * Neither the name of the Monash University nor the names of its contributors
**         may be used to endorse or promote products derived from this software
**         without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
** THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
** BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
** OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**
** Contact:
*%  Toan Nguyen- Toan.Nguyen(at)monash.edu
*%
*% Development Team :
*%  http://monash.edu/mivp
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "dzdisplay.h"

#include <mpi.h>
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <vector>
#include <math.h>

#include <omicron.h>

char keyOnce[GLFW_KEY_LAST + 1];
#define glfwGetKeyOnce(WINDOW, KEY)             \
    (glfwGetKey(WINDOW, KEY) ?              \
     (keyOnce[KEY] ? false : (keyOnce[KEY] = true)) :   \
     (keyOnce[KEY] = false))

using namespace omicron;
using namespace std;

static int numprocs;

Control_t gcontrol;

int cwidth, cheight;
double maxdownsample;
int buffersize = 16;
int numthreads = 2;

double zoom_amount = 0;
double zoom_factor = 8000;
double pan_amount = 0.01;

string str_system = "desktop";
vector<Img_t> filenames;

bool noomicron = false;
bool minimap = false;

// slide view: auto go to next slide in every slidetime seconds
bool slideview = false;
float slidetime = 10;
uint slidetime_last = 0;

ServiceManager* sm;

void usage()
{
	cout << endl;
	cout << "Usage: ./vsviewer [-h] [-s system] [-n] [-b buffersize] [-t numthreads] [-a seconds] [-i img1] [-l img2l img2r]" << endl;
	cout << "  -s: system {desktop, cave2}. Default: desktop" << endl;
	cout << "  -n: dont use Omicron (wand controller)" << endl;
	cout << "  -m: display minimap" << endl;
	cout << "  -h: print this help" << endl;
	cout << "  -b: bufer size. Default = 16" << endl;
	cout << "  -t: number of reading threads. Default = 2" << endl;
	cout << "  -a: go to next slide automatically after (a) seconds. Default = 10 seconds" << endl;
	cout << "  -i: single file" << endl;
	cout << "  -l: stereo, left first" << endl;
	cout << "  -r: stereo, right first" << endl;
	cout << "  need to provide at least 1 input file" << endl;
	cout << endl;
}

int initOmicron()
{
    DataManager* dm = DataManager::getInstance();
    char cCurrentPath[FILENAME_MAX];
    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
    {
        return errno;
    }
    dm->setCurrentPath(cCurrentPath);
    cout << "Current path: " << dm->getCurrentPath() << endl;

    Config* cfg = new Config("default.cfg");
    cfg->load();

    // Start services and begin listening to events.
    cout << "Connecting to tracking machine..." << endl;
    sm = new ServiceManager();
    sm->setupAndStart(cfg);

    return 0;
}

int initParameters(int argc, char* argv[], int myid)
{
	int i = 1;
	while (i < argc)
	{
		Img_t img;
		if (strcmp(argv[i],"-h")==0)
		{
			usage();
			exit(0);
		}
		else if (strcmp(argv[i],"-s")==0)
		{
			str_system = argv[++i];
			i++;
		}
		else if (strcmp(argv[i],"-n")==0)
		{
			noomicron = true;
			i++;
		}
		else if (strcmp(argv[i],"-m")==0)
		{
			minimap = true;
			i++;
		}
		else if (strcmp(argv[i], "-b")==0)
		{
			buffersize = atoi(argv[++i]);
			i++;
		}
		else if (strcmp(argv[i], "-t")==0)
		{
			numthreads = atoi(argv[++i]);
			i++;
		}
		else if (strcmp(argv[i], "-a")==0) {
			slidetime = atof(argv[++i]);
			i++;
		}
		else if (strcmp(argv[i],"-i")==0)
		{
			img.type = 0;
			img.filename1 = argv[++i];
			if(!Utils::fileExist(img.filename1.c_str()))
				return -1;
			filenames.push_back(img);
			i++;
		}
		else if (strcmp(argv[i],"-l")==0)
		{
			img.type = 1;
			img.filename1 = argv[++i];
			if( i >= argc-1)
				return -1;
			img.filename2 = argv[++i];
			if(!Utils::fileExist(img.filename1.c_str()) || !Utils::fileExist(img.filename2.c_str()))
				return -1;
			filenames.push_back(img);
			i++;
		}
		else if (strcmp(argv[i],"-r")==0)
		{
			img.type = 2;
			img.filename1 = argv[++i];
			if( i >= argc-1)
				return -1;
			img.filename2 = argv[++i];
			if(!Utils::fileExist(img.filename1.c_str()) || !Utils::fileExist(img.filename2.c_str()))
				return -1;
			filenames.push_back(img);
			i++;
		}
		else
			i++;
	}

	if(filenames.size() == 0)
	{
		cout << "No input files" << endl;
		return -1;
	}

	if(str_system.compare("desktop") == 0)
	{
		cwidth = 400;
		cheight = 800;
		pan_amount = 0.001;
	}
	else
	{
		cwidth = 1366;
		cheight = 3072;
	}

	cout << "(" << myid << ") System: " << str_system << endl;
	cout << "(" << myid << ") Input files: " << endl;
	for(int i=0; i < filenames.size(); i++)
	{
		if(filenames[i].type == 0)
	 		cout << "single: " << filenames[i].filename1 << endl;
	 	else
	 		cout << "stereo: " << filenames[i].filename1 << " " << filenames[i].filename2 << endl;
	}

	return 0;
}

int loadNextFile(DZDisplay* display, int& file_index, int numprocs, bool backward=false)
{
	if(filenames.size() > 1)
	{
		char buff_r[1024];
		MPI_Status stat; 

		if(backward)
		{
			file_index--;
			if(file_index < 0)
				file_index = filenames.size() - 1;
		}
		else
		{
			file_index++;
			if(file_index >= filenames.size())
				file_index = 0;
		}	
		display->loadVirtualSlide(filenames[file_index]);
		int64_t w, h;
		display->getLevel0Size(w, h);
		Utils::resetParameters(w, h, cwidth, cheight, numprocs, gcontrol);
		maxdownsample = display->getMaxDownsample();
		zoom_amount = w / zoom_factor > 0.5 ? w / zoom_factor : 0.5 ;
		display->display(0, 0);
		std::ostringstream ss;
		ss << "N " << file_index;
		cout << "Send: " << ss.str() << endl;
		for(int i=1;i<numprocs;i++)  
			MPI_Send((char*)ss.str().c_str(), 256, MPI_CHAR, i, 0, MPI_COMM_WORLD);
		for(int i=1;i<numprocs;i++)  
			MPI_Recv(buff_r, 256, MPI_CHAR, i, 0, MPI_COMM_WORLD, &stat);

		if(slideview) {
			slidetime_last = Utils::getTime();
		}
	}
	return 0;
}

#ifdef main
#  undef main
#endif
int main( int argc, char* argv[] ){

	char idstr[1024]; char buff[1024], buff_r[1024];  
	char processor_name[MPI_MAX_PROCESSOR_NAME];  
	int numprocs; int myid; int i; int namelen;  
	MPI_Status stat;  

	MPI_Init(&argc,&argv);  
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);  
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);  
	MPI_Get_processor_name(processor_name, &namelen);  

	if(initParameters(argc, argv, myid) != 0)
	{
		cout << "Error! Please check parameters." << endl;
		return -1;
	}
	
	if(myid == 0)  // server
	{
		int file_index = 0;

		DZDisplay* display = new DZDisplay(0, 800, 600, numprocs - 1);
		display->initDisplay();
	  	if(display->loadVirtualSlide(filenames[0]) != 0)
	  		return -1;
	  	//display->display(0, 0, 2);
	  	//double move_ratio = 800.0/((numprocs-1)*cwidth);

	  	int64_t w, h;
	  	display->getLevel0Size(w, h);
	  	Utils::resetParameters(w, h, cwidth, cheight, numprocs, gcontrol);
	  	maxdownsample = display->getMaxDownsample();
		zoom_amount = w / zoom_factor > 0.5 ? w / zoom_factor : 0.5 ;

	  	// wand service
	  	if(!noomicron)
	  		initOmicron();

	  	for(i=1;i<numprocs;i++)  
    	{  
      		MPI_Recv(buff, 256, MPI_CHAR, i, 0, MPI_COMM_WORLD, &stat);
      		printf("Client %d: %s\n", i, buff); 
      		if(strcmp(buff, "ready") != 0)
			{
				cout << "node: " << i << " failed!!" << endl;
				return 1;
			}
    	} 

    	//display
    	string msg = Utils::getParametersString(gcontrol, MODE_NORMAL);
        cout << "Send: " << msg << endl;
    	for(i=1;i<numprocs;i++)  
    		MPI_Send((char*)msg.c_str(), 256, MPI_CHAR, i, 0, MPI_COMM_WORLD);
    	for(i=1;i<numprocs;i++)  
    		MPI_Recv(buff_r, 256, MPI_CHAR, i, 0, MPI_COMM_WORLD, &stat);

    	uint t_start = Utils::getTime();
    	
    	//main loop
    	bool running = true;
    	bool need_full_update = false;
    	display->display();
    	while( running )
    	{
    		bool update = true;
			int mode = MODE_FAST;
			//display->display(40, 20, 28.0);
			glfwPollEvents();

			if( glfwGetKey(display->window, GLFW_KEY_ESCAPE ) == GLFW_PRESS ||
           		glfwWindowShouldClose(display->window) )
			{
				running = false;
          		strcpy(buff, "stop");
				for(i=1;i<numprocs;i++)  
					MPI_Send(buff, 256, MPI_CHAR, i, 0, MPI_COMM_WORLD);
				break;
			}

			if( glfwGetKey(display->window, GLFW_KEY_LEFT ) )
				Utils::pan(0, pan_amount*w/gcontrol.downsample, gcontrol);
		
			else if ( glfwGetKey(display->window, GLFW_KEY_RIGHT ) )
				Utils::pan(1, pan_amount*w/gcontrol.downsample, gcontrol);
		
			else if ( glfwGetKey(display->window, GLFW_KEY_UP ) )
				Utils::pan(2, pan_amount*h/gcontrol.downsample, gcontrol);
		
			else if ( glfwGetKey(display->window, GLFW_KEY_DOWN ) )
				Utils::pan(3, pan_amount*h/gcontrol.downsample, gcontrol);
		
			else if ( glfwGetKeyOnce(display->window, GLFW_KEY_X ) )
				Utils::zoom(w, h, cwidth, cheight, numprocs, maxdownsample, -zoom_amount, gcontrol);

			else if ( glfwGetKeyOnce(display->window, GLFW_KEY_Z ) )
				Utils::zoom(w, h, cwidth, cheight, numprocs, maxdownsample, zoom_amount, gcontrol);			
	
			else if ( glfwGetKeyOnce(display->window, GLFW_KEY_PAGE_UP ) )
			{
				loadNextFile(display, file_index, numprocs);
                display->display(0, 0, 2);
			}

			else if ( glfwGetKeyOnce(display->window, GLFW_KEY_PAGE_DOWN ) )
			{
				loadNextFile(display, file_index, numprocs, true);
                display->display(0, 0, 2);
			}

			else if ( glfwGetKeyOnce(display->window, GLFW_KEY_N ) ) // next file
			{
				loadNextFile(display, file_index, numprocs);
				display->display(0, 0, 2);
			}

			else if ( glfwGetKeyOnce(display->window, GLFW_KEY_B ) ) // previous file
			{
				loadNextFile(display, file_index, numprocs, true);
				display->display(0, 0, 2);
			}

			else if ( glfwGetKeyOnce(display->window, GLFW_KEY_A ) ) // slide view mode
			{
				slideview = !slideview;
				slidetime_last = Utils::getTime();
			}	
			
			else if ( glfwGetKeyOnce(display->window, GLFW_KEY_G ) )
				mode = MODE_HIGH; // high quality
			
			else
				update = false;

			if(update)
    		{
    			//display->display(gleft*move_ratio, gtop*move_ratio);
    			msg = Utils::getParametersString(gcontrol, mode);
    			cout << "Send: " << msg << endl;
    			for(i=1;i<numprocs;i++)  
					MPI_Send((char*)msg.c_str(), 256, MPI_CHAR, i, 0, MPI_COMM_WORLD);
				for(i=1;i<numprocs;i++)  
					MPI_Recv(buff_r, 256, MPI_CHAR, i, 0, MPI_COMM_WORLD, &stat);
				t_start = Utils::getTime();
				if(mode == MODE_FAST)
					need_full_update = true;
				// clear keyboard
    		}

			// Wand events
			if(!noomicron)
			{
				// Poll services for new events.
				sm->poll(); 
				// Get available events
				Event evts[OMICRON_MAX_EVENTS];
				int av = sm->getEvents(evts, OMICRON_MAX_EVENTS);
				for(int evtNum = 0; evtNum < av; evtNum++)
				{
					//ofmsg("Event received: timestamp %1%", %evts[evtNum].getTimestamp());
					if(evts[evtNum].getServiceType() == Service::Wand && evts[evtNum].getSourceId() == 1)
					{
						update = true;
						mode = MODE_FAST;

						switch(evts[evtNum].getFlags())
						{
							case Event::Button5: // L1
								if(evts[evtNum].getType() == Event::Down)
								{
									loadNextFile(display, file_index, numprocs);
									display->display(0, 0, 2);
								}
								break;
							case Event::Button7: // L2
								if(evts[evtNum].getType() == Event::Down)
								{
									loadNextFile(display, file_index, numprocs, true);
									display->display(0, 0, 2);
								}
								break;
							case Event::Button3: // B
								Utils::zoom(w, h, cwidth, cheight, numprocs, maxdownsample, zoom_amount, gcontrol);
								break;
							case Event::Button2: // A
								Utils::zoom(w, h, cwidth, cheight, numprocs, maxdownsample, -zoom_amount, gcontrol);
								break;
							case Event::Button4: // X
								slideview = !slideview;
								slidetime_last = Utils::getTime();
								break;					
							case Event::ButtonLeft:
								Utils::pan(0, pan_amount*w/gcontrol.downsample, gcontrol);
								break;
							case Event::ButtonRight:
								Utils::pan(1, pan_amount*w/gcontrol.downsample, gcontrol);
								break;	
							case Event::ButtonUp:
								Utils::pan(2, pan_amount*h/gcontrol.downsample, gcontrol);
								break;
							case Event::ButtonDown:
								Utils::pan(3, pan_amount*h/gcontrol.downsample, gcontrol);
								break;
							default:
								update = false;
								break;
						}
						if(update)
	            		{
	            			//display->display(gleft*move_ratio, gtop*move_ratio);
	            			msg = Utils::getParametersString(gcontrol, mode);
	            			cout << "Send: " << msg << endl;
	            			for(i=1;i<numprocs;i++)  
	    						MPI_Send((char*)msg.c_str(), 256, MPI_CHAR, i, 0, MPI_COMM_WORLD);
	    					for(i=1;i<numprocs;i++)  
	    						MPI_Recv(buff_r, 256, MPI_CHAR, i, 0, MPI_COMM_WORLD, &stat);
	    					t_start = Utils::getTime();
	    					need_full_update = true;
	            		}
					}
				}
			}	

			if(slideview) {
        		if(Utils::getTime() - slidetime_last > slidetime * 1000.0) {
        			loadNextFile(display, file_index, numprocs);
                	display->display(0, 0, 2);

                	t_start = Utils::getTime();
                	need_full_update = true;
        		}
        	}

			if(need_full_update)
        	{
        		uint timediff = Utils::getTime() - t_start;
        		if(timediff > 500)
        		{
        			msg = Utils::getParametersString(gcontrol, MODE_NORMAL);
            		cout << "Send: " << msg << endl;
        			for(i=1;i<numprocs;i++)  
    					MPI_Send((char*)msg.c_str(), 256, MPI_CHAR, i, 0, MPI_COMM_WORLD);
    				for(i=1;i<numprocs;i++)  
    					MPI_Recv(buff_r, 256, MPI_CHAR, i, 0, MPI_COMM_WORLD, &stat);
    				need_full_update = false;
        		}
        	}
        	
    	}
	}  
	else  // clients
	{  
		DZDisplay* display = new DZDisplay(myid, cwidth, cheight, numprocs - 1, numthreads);
		display->setBufferSize(buffersize);
		display->initDisplay();
	  	if(display->loadVirtualSlide(filenames[0]) == -1)
	  		return -1;

		int mode;
	  	int64_t l, t;
	  	double ds;

	  	strcpy(buff, "ready");
		MPI_Send(buff, 256, MPI_CHAR, 0, 0, MPI_COMM_WORLD);

		bool running = true;
		while(running)
    	{
    		MPI_Recv(buff, 256, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &stat); 
    		//cout << "(" << myid << ") Rev: " << buff << endl;
			if (strcmp(buff, "stop") == 0)
			{
				running = false;
			}
			else if(buff[0] == 'U')
			{
				Utils::parseParametersString(buff, mode, l, t, ds);
				display->display(l, t, ds, mode, minimap);

				if(myid == 9)
					cout << "(9) Read: " << display->read_time << " render: " << display->render_time << endl;

				strcpy(buff, "done");
				MPI_Send(buff, 256, MPI_CHAR, 0, 0, MPI_COMM_WORLD); 
			}
			else if(buff[0] == 'N')
			{
				int file_index;
				istringstream iss(buff);
				string sub;
				iss >> sub; 
				iss >> sub; file_index = atoi(sub.c_str());
				if(display->loadVirtualSlide(filenames[file_index]) == -1)
					return -1;

				strcpy(buff, "done");
				MPI_Send(buff, 256, MPI_CHAR, 0, 0, MPI_COMM_WORLD); 
			}
    	}
	}  

	MPI_Finalize(); 

	return 0;
}
