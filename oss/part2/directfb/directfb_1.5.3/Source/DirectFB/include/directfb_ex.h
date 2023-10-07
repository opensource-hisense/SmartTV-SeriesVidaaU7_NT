
#ifndef __DIRECTFB_EX_H__

#define __DIRECTFB_EX_H__


typedef struct DFBSocketEvent
{
    unsigned int m_Type; //0:key 1:mouse rel 2:mouse abs 3:mouse button 4: discard or not 5: set discard key exception 6: unset discard key exception

	union
	{
	    struct
	    {
	        unsigned int m_Type; //0:press 1:release
			unsigned int m_KeySymbol;
			unsigned int m_KeyCode;
	    }Key;

		struct
		{
		    int m_XRef;
			int m_YRef;
		}MouseRelMotion;

		struct
		{
		    int m_XPos;
			int m_YPos;
			int m_XMax;
			int m_YMax;
		}MouseABSMotion;

		struct
		{
		    unsigned int m_Type; //0:press 1://release 
		    unsigned int m_Status; //0:left 1:right 2://middle
		}Button;

		struct
		{
		    int m_Discard;
		}KeyState;
		
		struct
		{
		    unsigned int m_KeySymbol;
		}KeyException;
	}u;
}
DFBSocketEvent;

#endif
