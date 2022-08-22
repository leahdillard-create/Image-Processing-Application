
/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Nathan Palmer

- All project requirements fully met? (YES or NO):
    <Yes>

- If no, please explain what you could not get to work:

- Did you do any optional enhancements? If so, please explain:
    <ANSWER>
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image)
{
    int num_rows = image.size();//rows = height
    int num_columns = image[0].size();// colums = width
    
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    
    for (int row = 0; row<num_rows; row++)
    {
        for (int col = 0; col< num_columns; col++)
        {
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;
            
            double distance = sqrt(pow((col - (num_columns/2)),2)+pow((row - (num_rows/2)),2));
            double scaling_factor = (num_rows - distance)/num_rows;
            int newred = red_color*scaling_factor;
            int newblue =  blue_color*scaling_factor;
            int newgreen = green_color*scaling_factor;
            
            new_image[row][col].blue = newblue;
            new_image[row][col].green = newgreen;
            new_image[row][col].red = newred;
        }
    }

    return new_image;
}

vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image, double scaling_factor)
{
    int num_rows = image.size();//rows = height
    int num_columns = image[0].size();// colums = width
    int newred;
    int newgreen;
    int newblue;
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    for (int row = 0; row<num_rows; row++)
    {
        for (int col = 0; col< num_columns; col++)
        {
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;
            int average = (blue_color + green_color + red_color)/3;
            
            if (average >= 170){
                newred= (255-(255-red_color)*scaling_factor);
                newgreen= (255-(255-green_color)*scaling_factor);
                newblue= (255-(255-blue_color)*scaling_factor);
            }
            else if (average < 90){
                newred = red_color *scaling_factor;
                newgreen = green_color*scaling_factor;
                newblue = blue_color*scaling_factor;
            }
            else{
                newred = red_color;
                newgreen = green_color;
                newblue = blue_color;
            }
            new_image[row][col].blue = newblue;
            new_image[row][col].green = newgreen;
            new_image[row][col].red = newred;
        }
    }
    return new_image;
}
vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image)
{
    //width=img.getWidth()
    //height=img.getHeight()
    int num_rows = image.size();//rows = height
    int num_columns = image[0].size();// colums = width
    
    //newimg = image.EmptyImage(width,height)
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    
    // for row in range(height):
        //for col in range(width):
    for (int row = 0; row<num_rows; row++)
    {
        for (int col = 0; col< num_columns; col++)
        {
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;
            
            int grey = (blue_color+green_color+red_color)/3;
            
            new_image[row][col].blue = grey;
            new_image[row][col].green = grey;
            new_image[row][col].red = grey;
        }
    }
    return new_image;
}
vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image)
{
    int num_rows = image.size();//rows=height
    int num_columns = image[0].size();
    
    vector<vector<Pixel>> new_image(num_columns, vector<Pixel> (num_rows));

    for(int i=0;i<num_columns;i++){
        for(int j=0;j<num_rows;j++){
            new_image[i][j]=image[num_rows-1-j][i];
        }
    }
    
    return new_image;
} 

vector<vector<Pixel>> rotateby90(const vector<vector<Pixel>>& image){
    int num_rows = image.size();//rows=height
    int num_columns = image[0].size();
    
    vector<vector<Pixel>> new_image(num_columns, vector<Pixel> (num_rows));

    for(int i=0;i<num_columns;i++){
        for(int j=0;j<num_rows;j++){
            new_image[i][j]=image[num_rows-1-j][i];
        }
    }
    
    return new_image;
}

vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image, int number){
    int num_rows = image.size();//rows=height
    int num_columns = image[0].size();// colums=width
    
    vector<vector<Pixel>> new_image(num_columns, vector<Pixel> (num_rows));
    
    int angle = number*90;
    if (angle%90!=0){
        cout<<"Angle must be multiple of 90 degrees" << endl;
    }
    else if(angle%360==0){
        new_image = image;
    }
    else if(angle%360==90){
        new_image = rotateby90(image);
    }
    else if(angle%360==180){
        new_image = rotateby90(rotateby90(image));
    }
    else{
         new_image= rotateby90(rotateby90(rotateby90(image)));
    }
    return new_image;
}

vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int x_scale, int y_scale){
    double num_rows = image.size();//rows=height
    double num_columns = image[0].size();//colums=width
    vector<vector<Pixel>> new_image(num_rows*y_scale, vector<Pixel> (num_columns*x_scale));
    
       for(int row=0; row<(num_rows*y_scale); row++){
            for(int col=0; col<(num_columns*x_scale); col++){ 
                double col_scale = col/x_scale;
                double row_scale = row/y_scale;
                new_image[row][col]=image[row/y_scale][col/x_scale];
        }
    }
    
    return new_image;
}
vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image){
    
    int num_rows = image.size();//rows = height
    int num_columns = image[0].size();// colums = width
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    
    for (int row = 0; row<num_rows; row++)
    {
        for (int col = 0; col< num_columns; col++)
        {

            int newred, newgreen, newblue;
            
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;
            
            int grey = (blue_color+green_color+red_color)/3;
            
            if(grey >= 255/2){
                newred = 255;
                newgreen = 255;
                newblue =255;
            }
            else{
                newred=0;
                newgreen=0;
                newblue=0;
            }    
            
            new_image[row][col].blue = newblue;
            new_image[row][col].green = newgreen;
            new_image[row][col].red = newred;
        }
    }
    return new_image;
}

vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image, double scaling_factor){
    
    int num_rows = image.size();//rows = height
    int num_columns = image[0].size();// colums = width
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    
    for (int row = 0; row<num_rows; row++)
    {
        for (int col = 0; col< num_columns; col++)
        {

            double newred, newgreen, newblue;
            
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;
                            
            newblue= (255-(255-blue_color)*scaling_factor);
            newgreen= (255-(255-green_color)*scaling_factor);
            newred= (255-(255-red_color)*scaling_factor);
            
            new_image[row][col].blue = newblue;
            new_image[row][col].green = newgreen;
            new_image[row][col].red = newred;
        }
    }
    return new_image;
}

vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image, double scaling_factor){
    
    int num_rows = image.size();//rows = height
    int num_columns = image[0].size();// colums = width
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    
    for (int row = 0; row<num_rows; row++)
    {
        for (int col = 0; col< num_columns; col++)
        {

            double newred, newgreen, newblue;
            
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;
                            
            newblue= (blue_color*scaling_factor);
            newgreen= (green_color*scaling_factor);
            newred= (red_color*scaling_factor);
            
            new_image[row][col].blue = newblue;
            new_image[row][col].green = newgreen;
            new_image[row][col].red = newred;
        }
    }
    return new_image;
}

vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image){
    
    int num_rows = image.size();//rows = height
    int num_columns = image[0].size();// colums = width
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    
    for (int row = 0; row<num_rows; row++)
    {
        for (int col = 0; col< num_columns; col++)
        {
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;
            int red_color = image[row][col].red;

            double newred, newgreen, newblue, max_color;
           if(green_color > blue_color && green_color>red_color){
               max_color = green_color;
           }
           if(blue_color > green_color && blue_color>red_color){
               max_color = blue_color;
           } 
            
           if(red_color > green_color && red_color>blue_color){
               max_color = red_color;
           }  
            
   
            
            if(red_color + green_color + blue_color >= 550){
                newred = 255;
                newgreen = 255;
                newblue = 255;
                
            }
            else if(red_color + green_color + blue_color <= 150){
                
                newred = 0;
                newgreen = 0;
                newblue = 0;
            }
            else if(max_color == red_color){
                
                newred = 255;
                newgreen = 0;
                newblue = 0;
            }
            else if(max_color == green_color){
                
                newred = 0;
                newgreen = 255;
                newblue = 0;
            }
            else{
                
                newred = 0;
                newgreen = 0;
                newblue = 255;
            }
            
            new_image[row][col].blue = newblue;
            new_image[row][col].green = newgreen;
            new_image[row][col].red = newred;
            
        }
    }
    return new_image;
}

int main()
{
    int input;
    string filename;
    string new_file;
    bool quit=false;
    cout << endl << "Image Processing Application" << endl;
    cout << "Enter input BMP filename: ";
    cin >> filename;
    vector<vector<Pixel>> original_image = read_image(filename);
    do{
        
        cout << endl;
        cout<<"IMAGE PROCESSING MENU" << endl;
        cout <<"0) EXIT " << endl;
        cout<<"1) Vignette"<<endl;
        cout<< "2) Clarendon" << endl;
        cout << "3) Grayscale" << endl;
        cout << "4) Rotate 90 degrees" << endl;
        cout << "5) Rotate multiple 90 degrees" << endl;
        cout << "6) Enlarge" << endl;
        cout << "7) High contrast" << endl;
        cout << "8) Lighten"<< endl;
        cout << "9) Darken"<< endl;
        cout << "10) Black, white, red, green, blue"<< endl;
        cout<< "11) Change image (current: "<< filename << ")" << endl;
        cout << endl;
        cout << "Enter menu selection (0 to quit): ";
        cin >> input;
        cout << endl;
        
        if(input==11){
                cout << "Enter input BMP filename: ";
                cin >> filename;
                original_image = read_image(filename);
                cout<<"Successfully changed input image!"<<endl;
        }
        
        else{
        switch(input){
            
            case 0:{
                cout << "Thank you for using my program!" << endl;
                cout << "Quitting..." << endl;
                break;
            }
            case 1:{
                cout << "Vignette selected" << endl;
                vector<vector<Pixel>> new_image = process_1(original_image);
                cout<< "Enter output BMP filename: ";
                cin >> new_file;
                bool success = write_image(new_file, new_image);
                if(success==true){
                    cout<< "Sucessfully applied vignette!"<< endl;
                    continue;
                }
                else{
                    cout <<"Could not write image to new file." << endl;
                    break;
                }
                break;
            }
            case 2:{
                double scaling_factor;
                cout<< "Clarendon selected"<< endl;
                vector<vector<Pixel>> new_image = process_2(original_image, scaling_factor);
                cout<< "Enter output BMP filename: ";
                cin >> new_file;
                cout <<"Enter scaling factor: ";
                cin >> scaling_factor;
                bool success = write_image(new_file, new_image);
                if (success == true){
                    cout<< "Sucessfully applied clarendon!"<< endl;
                }
                else{
                    cout <<"Could not write image to new file." << endl;
                    break;
                }
                break;
            }
            case 3:{
                cout<< "Grayscale selected"<< endl;
                vector<vector<Pixel>> new_image = process_3(original_image);
                cout<< "Enter output BMP filename: ";
                cin >> new_file;
                bool success = write_image(new_file, new_image);
                if(success==true){
                    cout<< "Sucessfully applied grayscale!"<< endl;
                    continue;
                }
                else{
                   cout <<"Could not write image to new file." << endl;
                    break;
                }
                break;
            }
            case 4:{
                cout<< "Rotate 90 degrees selected"<< endl;
                vector<vector<Pixel>> new_image = process_4(original_image);
                cout<< "Enter output BMP filename: ";
                cin >> new_file;
                bool success = write_image(new_file, new_image);
                if(success==true){
                    cout<< "Successfully applied 90 degree rotation!"<< endl;
                    continue;
                }
                else{
                   cout <<"Could not write image to new file." << endl;
                    break;
                }
                break;
            }
            case 5:{
                int multiple=0;
                cout<< "Rotate multiple 90 degrees selected"<< endl;
                cout<< "Enter output BMP filename: ";
                cin >> new_file;
                cout <<"Enter number of 90 degree rotations: ";
                cin >> multiple;
                vector<vector<Pixel>> new_image = process_5(original_image,multiple);
                bool success = write_image(new_file, new_image);
                if(success==true){
                    cout<< "Successfully applied multiple 90 degree rotations!"<< endl;
                    continue;
                }
                else{
                    cout <<"Could not write image to new file." << endl;
                    break;
                }
                break;
            }
            case 6:{
                double x,y;
                cout<< "Enlarge selected"<< endl;
                cout<< "Enter output BMP filename: ";
                cin >> new_file;
                cout << "Enter X scale: ";
                cin >> x;
                cout << "Enter Y scale: ";
                cin >> y;
                vector<vector<Pixel>> new_image = process_6(original_image, x, y);
                bool success = write_image(new_file, new_image);
                if(success==true){
                    cout<< "Successfully enlarged!"<< endl;
                    continue;
                }
                else{
                    cout <<"Could not write image to new file." << endl;
                    break;
                }
                break;
            }
            case 7:{
                cout<< "High contrast selected"<< endl;
                vector<vector<Pixel>> new_image = process_7(original_image);
                cout<< "Enter output BMP filename: ";
                cin >> new_file;
                bool success = write_image(new_file, new_image);
                if(success==true){
                    cout<< "Sucessfully applied high contrast!"<< endl;
                    continue;
                }
                else{
                    cout <<"Could not write image to new file." << endl;
                    break;
                }
                break;
            }
            case 8:{
                double scale=0;
                cout<< "Lighten selected"<< endl;
                cout<< "Enter output BMP filename: ";
                cin >> new_file;
                cout <<"Enter scaling factor: ";
                cin >> scale;
                vector<vector<Pixel>> new_image = process_8(original_image, scale);
                bool success = write_image(new_file, new_image);
                if(success==true){
                    cout<< "Sucessfully lightened!"<< endl;
                    continue;
                }
                else{
                    cout <<"Could not write image to new file." << endl;
                    break;
                }
                break;
            }
            case 9:{
                double scale=0;
                cout<< "Darken selected"<< endl;
                cout<< "Enter output BMP filename: ";
                cin >> new_file;
                cout <<"Enter scaling factor: ";
                cin >> scale;
                vector<vector<Pixel>> new_image = process_9(original_image,scale);
                bool success = write_image(new_file, new_image);
                if(success==true){
                    cout<< "Sucessfully darkened!"<< endl;
                    continue;
                }
                else{
                    cout <<"Could not write image to new file." << endl;
                    break;
                }
                break;
            }
            case 10:{
                cout<< "Black, white, red, green, blue selected"<< endl;
                vector<vector<Pixel>> new_image = process_10(original_image);
                cout<< "Enter output BMP filename: ";
                cin >> new_file;
                bool success = write_image(new_file, new_image);
                if(success==true){
                    cout << "Successfully applied black, white, red, green, blue filter!"<<endl;
                    continue;
                }
                else{
                    cout <<"Could not write image to new file." << endl;
                    break;
                }
                break;
            }
            case 11:{
                break;
            }
            default:{
                cout << "Input invalid, please enter a number 0-11."<<endl;
                continue;
                //quit=false;
            }
        }
        }
    }
    while(input&&!quit);

    return 0;
}