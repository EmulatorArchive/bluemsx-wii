
#include "GuiContainer.h"
#include "image_container.h"

#define TILE_SIZE 12

using namespace wsp;

GuiContainer::GuiContainer( int posx,  int posy, int width, int height, int alpha)
{
	cols = (width + TILE_SIZE/2) /TILE_SIZE;
	rows = (height + TILE_SIZE/2) / TILE_SIZE;
	win_layer = new TiledLayer(cols+1,rows+1,0);
	win_img.LoadImage(image_container);
	win_layer->SetStaticTileset(&win_img, TILE_SIZE, TILE_SIZE);
    win_layer->SetTransparency(alpha);
	SetPosition(posx, posy);
	Render();
}

GuiContainer::~GuiContainer()
{
    delete win_layer;
}

void GuiContainer::SetPosition( int posx,  int posy)
{
	x=posx;
	y=posy;
	win_layer->SetPosition(x - TILE_SIZE, y - TILE_SIZE);
}

int GuiContainer::GetWidth(void)
{
    return cols * TILE_SIZE;
}

int GuiContainer::GetHeight(void)
{
    return rows * TILE_SIZE;
}

void GuiContainer::SetVisible(bool state)
{
	win_layer->SetVisible(state);
}

Layer *GuiContainer::GetLayer(void)
{
    return win_layer;
}

void GuiContainer::Render(void)
{
	for(int y = 1; y <= rows; y++) {
        for(int x = 1; x <= cols; x++) {
            if( x == 1 && y == 1 )
        		win_layer->SetCell(x, y, 1);
            else if( x == 1 && y == rows )
        		win_layer->SetCell(x, y, 7);
            else if( x == cols && y == 1 )
        		win_layer->SetCell(x, y, 3);
            else if( x == cols && y == rows )
        		win_layer->SetCell(x, y, 9);
            else if( x == 1 )
        		win_layer->SetCell(x, y, 4);
            else if( x == cols )
        		win_layer->SetCell(x, y, 6);
            else if( y == 1 )
        		win_layer->SetCell(x, y, 2);
            else if( y == rows )
        		win_layer->SetCell(x, y, 8);
            else
        		win_layer->SetCell(x, y, 5);
		}
	}
}

