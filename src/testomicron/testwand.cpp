#include <iostream>
#include <omicron.h>

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif

using namespace omicron;
using namespace std;

int main( int argc, char* argv[] ){

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
	ServiceManager* sm = new ServiceManager();
	sm->setupAndStart(cfg);

	/*
	Event::Flags testButton = Event::Button7;
	Setting& s = cfg->lookup("config/wand");
	String navbtn = Config::getStringValue("testButton", s, "Button7");
    testButton = Event::parseButtonName(navbtn);
	*/

	while(true)
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
				switch(evts[evtNum].getFlags())
				{
					case Event::Button3:
						cout << "X" << endl;
						break;
					case Event::Button2:
						cout << "O" << endl;
						break;					
					case Event::ButtonLeft:
						cout << "LEFT" << endl;
						break;
					case Event::ButtonRight:
						cout << "RIGHT" << endl;
						break;	
					case Event::ButtonUp:
						cout << "UP" << endl;
						break;
					case Event::ButtonDown:
						cout << "DOWN" << endl;
						break;
				}

				float x = evts[evtNum].getExtraDataFloat(0);
    			float y = evts[evtNum].getExtraDataFloat(1);
    			// Thresholds
    			if(x < 0.1f && x > -0.1f) x = 0;
    			if(y < 0.1f && y > -0.1f) y = 0;
    			if(x > 0 || y > 0)
    				cout << "Extra data: " << x << " " << y << endl;
			}
		}
	}

	cout << "Done!" << endl;

	return 0;
}