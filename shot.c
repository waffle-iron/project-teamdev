/*
 * shot.c
 *
 *  Created on: 22.03.2012
 *      Author: anna
 */
#include "shot.h"
#include <string.h>

/*Function writes XImage structure contents as a BMP image to the specified file*/
void saveXImageToBitmap(XImage *pImage, char* full_filename)
{
	BITMAPFILEHEADER bmpFileHeader;
	BITMAPINFOHEADER bmpInfoHeader;
	FILE *fp;
	int dummy;
	void*data_ptr=NULL;
	int row_length;

	/*Setting all meanings to null*/
	memset(&bmpFileHeader, 0, sizeof(BITMAPFILEHEADER));
	memset(&bmpInfoHeader, 0, sizeof(BITMAPINFOHEADER));

	/*Initializing header structures*/
	bmpFileHeader.bfType = 0x4D42;
	bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +  pImage->width*pImage->height*4;
	bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpFileHeader.bfReserved1 = 0;
	bmpFileHeader.bfReserved2 = 0;

	bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfoHeader.biWidth = pImage->width;
	bmpInfoHeader.biHeight = pImage->height;
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = 32;
	dummy = (pImage->width * 3) % 4;
	if((4-dummy)==4)
		dummy=0;
	else
		dummy=4-dummy;
	bmpInfoHeader.biSizeImage = ((pImage->width*3)+dummy)*pImage->height;
	bmpInfoHeader.biCompression = 0;
	bmpInfoHeader.biXPelsPerMeter = 0;
	bmpInfoHeader.biYPelsPerMeter = 0;
	bmpInfoHeader.biClrUsed = 0;
	bmpInfoHeader.biClrImportant = 0;

	/*Opening file*/
	fp = fopen(full_filename,"wb");

	if(fp == NULL)
		return;

	/*Writing info*/
	fwrite(&bmpFileHeader, sizeof(bmpFileHeader), 1, fp);
	fwrite(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, fp);

	/*Writing image data*/
	row_length=4*pImage->width;
	data_ptr=pImage->data+row_length*((pImage->height)-1);
	while (data_ptr!=pImage->data)
	{
		fwrite(data_ptr,row_length,1,fp);
		data_ptr-=row_length;
	}
	fwrite(data_ptr,row_length,1,fp);

	/*Closing*/
	fclose(fp);
}

/*Function returns a pointer to a full-screen dump (XImage structure)*/
XImage* fullScreenCapture()
{
	int screen_num;
	Display *dpy;
	XImage *image;
	/*Connecting to X server*/
	dpy = XOpenDisplay(NULL);
	if (!dpy)
	{
		fprintf(stderr, "Error: Unable to establish connection to X server\n");
		return NULL;
	}
	/*Getting default screen*/
	screen_num = DefaultScreen(dpy);

	/*Doing a full-screen dump*/
	image = XGetImage (dpy, RootWindow(dpy, screen_num), 0, 0, DisplayWidth(dpy,screen_num),  DisplayHeight(dpy,screen_num), AllPlanes, ZPixmap);

	if (!image) {
		fprintf (stderr, "Error: Unable to get image from X server\n");
		XCloseDisplay(dpy);
		return NULL;
		}
	/*Finishing*/
	XCloseDisplay(dpy);
	return image;
}

/*Function creates a dump of the specified rectangle and returns a pointer to XImage struct
 * Parameters: x and y coordinates of the top left corner
 * 				width and height of the rectangle*/
XImage* rectangleCapture(int x_top_left, int y_top_left, int width, int height){
	int screen_num;
		Display *dpy;
		XImage *image;

		/*Connecting to X server*/
		dpy = XOpenDisplay(NULL);
		if (!dpy)
		{
			fprintf(stderr, "Error: Unable to establish connection to X server\n");
			return NULL;
		}
		/*Getting default screen*/
		screen_num = DefaultScreen(dpy);
		/*Doing a dump*/
		image = XGetImage (dpy, RootWindow(dpy, screen_num), x_top_left, y_top_left, width,  height, AllPlanes, ZPixmap);
		if (!image) {
			fprintf (stderr, "Error: Unable to get image from X server\n");
			XCloseDisplay(dpy);
			return NULL;
			}
		/*Finishing*/
		XCloseDisplay(dpy);
		return image;
}
/*
 * Function for getting active window handler
 * Returns window handler or -1 if fails*/
Window getActiveWindowHandler()
{
	Display  *display;
		Atom     actual_type;
		int      actual_format;
		long     nitems;
		long     bytes;
		unsigned long     *data;
		int      status;
		Window res=-1;
				/*Connecting to X server*/
		        display = XOpenDisplay(NULL);
		        if (!display) {
		                fprintf(stderr,"Error:Unable to connect to display\n");
		                return -1;
		        }
		        /*Fetching active window handler as a property of window manager*/
		        status = XGetWindowProperty(
		                display,
		                RootWindow(display, 0),
		                XInternAtom(display, "_NET_ACTIVE_WINDOW", True),
		                0,
		                (~0L),
		                False,
		                AnyPropertyType,
		                &actual_type,
		                &actual_format,
		                &nitems,
		                &bytes,
		                (unsigned char**)&data);

		        if (status != Success) {
		                fprintf(stderr, "Error: Getting active window was not successful\n");
		                XCloseDisplay(display);
		                return -1;
		        }
		        /*Finishing*/
		        if(nitems==1)res=data[0];
		        free(data);
		        XCloseDisplay(display);
		        return res;

}
/*Function gets handler for visible window with specified name
 * Parameters: full window name
 * Returns window handler or -1 if fails*/
Window getWindowHandler(char* window_name){
	Display  *display;
	Atom     actual_type;
	int      actual_format;
	long     nitems;
	long     bytes;
	unsigned long     *data;
	int      status;
	int i;
	char*name;
	Window res=-1;
	/*Connecting to X server*/
	display = XOpenDisplay(NULL);
	if (!display) {
			fprintf(stderr,"Error: Unable to connect to display\n");
			return -1;
	}
	/*Fetching all windows as a property of window manager*/
	status = XGetWindowProperty(
			display,
			RootWindow(display, 0),
			XInternAtom(display, "_NET_CLIENT_LIST_STACKING", True),
			0,
			(~0L),
			False,
			AnyPropertyType,
			&actual_type,
			&actual_format,
			&nitems,
			&bytes,
			(unsigned char**)&data);
	if (status != Success) {
			fprintf(stderr, "Error: Getting visible windows was not successful\n");
			return -1;
	}
	/*Looking for window with the specified name*/
	for (i=0; i < nitems; i++){
		XFetchName(display,data[i],&name);
		if((name!=NULL)&&(strcmp(name,window_name)==0)){
			res=data[i];
		}
		free(name);
	}
	/*Finishing*/
	free(data);
	XCloseDisplay(display);
	return res;

}
/*
 * Function fetches names of all windows on the screen
 * Parameters: all fetched window names(return parameter)
 * 				number of fetched window names
 * Returns 0 if ok, -1 if fails
 * */
int getWindowsNames(char*** names, int*size){
	Display  *display;
		Atom     actual_type;
		int      actual_format;
		long     nitems;
		long     bytes;
		unsigned long     *data;
		int      status;
		int i;
		char*name;
		char** tmp;
		int counter;
		/*Connecting to X server*/
	display = XOpenDisplay(NULL);
	if (!display) {
			fprintf(stderr,"Error:Unable to connect to X server\n");
			return -1;
	}
	/*Fetching all windows as the property of window manager*/
	status = XGetWindowProperty(
			display,
			RootWindow(display, 0),
			XInternAtom(display, "_NET_CLIENT_LIST_STACKING", True),
			0,
			(~0L),
			False,
			AnyPropertyType,
			&actual_type,
			&actual_format,
			&nitems,
			&bytes,
			(unsigned char**)&data);
	if (status != Success) {
			fprintf(stderr, "Error: Getting visible windows was not successful\n");
			return -1;
	}
	/*Getting window names*/
	tmp=(char**)malloc(nitems*sizeof(char*));
	counter=0;
	for (i=0; i < nitems; i++){
		XFetchName(display,data[i],&name);
		if(name!=0){
			tmp[counter]=name;
			counter+=1;
		}
	}
	free(data);
	/*Filling return parameters*/
	*size=counter;
	if(counter==0){
		fprintf(stderr,"Warning: no windows with names found\n");
		*names=NULL;
		free(tmp);
		XCloseDisplay(display);
		return -1;
	}
	*names=(char**)malloc(counter*sizeof(char*));
	for(i=0;i<counter;i++){
		(*names)[i]=tmp[i];
	}
	/*Finishing*/
	free(tmp);
	XCloseDisplay(display);
	return 0;
}
/*
 * Checks if the window is visible on the screen
 * Returns 1 as true, 0 as false, -1 if failed*/
int isVisible(Window w){
	Display  *display;
	Atom hidden;
	Atom     actual_type;
	int      actual_format;
	long     nitems;
	long     bytes;
	int      status;
	int 	i;

	Atom* props;

	/*Connecting to X server*/
	display = XOpenDisplay(NULL);
	if (!display) {
			fprintf(stderr,"Error:Unable to connect to X server\n");
			return -1;
	}
	/*Creating identifier for hidden property*/
	hidden = XInternAtom(display, "_NET_WM_STATE_HIDDEN", True);
	/*Fetching window state properties*/
	status = XGetWindowProperty(
			display,
			w,
			XInternAtom(display, "_NET_WM_STATE", True),
			0,
			(~0L),
			False,
			AnyPropertyType,
			&actual_type,
			&actual_format,
			&nitems,
			&bytes,
			(unsigned char**)&props);
	if (status != Success) {
			fprintf(stderr, "Error: Getting window state was not successful\n");
			return -1;
	}
	/*Searching hidden property among properties*/
	for(i=0;i<nitems;i++){
		if (props[i]==hidden){
			/*Window is not visible*/
			XFree(props);
			XCloseDisplay(display);
			return 0;
		}

	}
	/*Finishing*/
	XFree(props);
	XCloseDisplay(display);
	return 1;
}
/*
 * Function for window capturing
 * Parameters: a window handler for window to capture
 * Returns a ponter to created dump(XImage struct)*/
XImage* windowCapture(Window handler){
			Display *dpy;
			XImage *image;
			XWindowAttributes attr;
			int visibility=isVisible(handler);
			if(visibility==0){
				fprintf(stderr,"Error: Window %d is not visible\n",(int)handler);
				return NULL;
			}
			if(visibility==-1){
				fprintf(stderr,"Error: Failed fetching window visibility for %d\n", (int)handler);
				return NULL;
			}
			/*Connecting to X server*/
			dpy = XOpenDisplay(NULL);
			if (!dpy)
			{
                fprintf(stderr,"Error: Unable to connect to X server\n");
				return NULL;
			}
			/*Getting window attributes*/
			XGetWindowAttributes(dpy,handler,&attr);

			/*Doing a dump*/
			image = XGetImage (dpy, handler, 0, 0, attr.width,  attr.height, AllPlanes, ZPixmap);
			if (!image) {
				fprintf (stderr, "Error: Unable to get image from X server\n");
				XCloseDisplay(dpy);
				return NULL;
				}
			/*Finishing*/
			XCloseDisplay(dpy);
			return image;
}

