#include "bsod.h"
#include "simple.h"
#include "common.h"

void bsod_frame()
{
    static const char *HEX_Digits = "0123456789ABCDEF";
    struct event e;
    e=event_get();
    while (e.type != no_event) 
    {
        switch(e.type)
        {
        case evt_keyboard_press: 
            switch (e.kbd.key)
            {
            case 0x52:
                move_cursor_up();
                break;
            case 0x51:
                move_cursor_down();
                break;
            case 0x50:
                move_cursor_left();
                break;
            case 0x4F:
                move_cursor_right();
                break;
            default:
                if (e.kbd.key < 100)
                {
                    if (e.kbd.key == 42) // backspace
                    {
                        move_cursor_left();
                        vram[cursor.y][cursor.x] = 32;
                    }
                    else if (e.kbd.key == 40) // enter
                    {
                        //move_cursor_return();
                        // special behavior here...
                    }
                    else
                    {
                        vram[cursor.y][cursor.x] = e.kbd.sym;
                        move_cursor_right();
                    }
                }
                message("pressed %d with mod %d\n", e.kbd.key, e.kbd.mod);
                print_at(31,18,"KB pressed      ");
                vram[18][45]=HEX_Digits[(e.kbd.key>>4) & 0xF];
                vram[18][46]=HEX_Digits[e.kbd.key&0xf];
            }
            break;

        case evt_keyboard_release: 
            print_at(31,18,"KB released     ");
            vram[18][45]=HEX_Digits[(e.kbd.key>>4) & 0xF];
            vram[18][46]=HEX_Digits[e.kbd.key&0xf];
            break;

        case evt_device_change:
            // It seems the disconnect event is not sent currently...
            if (e.device.type == device_unconnected)
                print_at(31, 18, "dev. unconnected");
            else if (e.device.type == device_keyboard)
                print_at(31, 18, "keyboard found! ");
            break;

        case evt_user:
            print_at(31, 18, "user event      ");
            break;
        
        default:
            print_at(31, 18, "UNHANDLED       ");
        }
        e=event_get();
    }
}

void bsod_init()
{
    // init palette
    palette[0]=RGB(0,255,255)<<16|RGB(0,0,128); // cyan on blue
    palette[1]=RGB(255,150,0)<<16|RGB(255,255,0); // orange fg, yellow bg
    palette[2]=RGB(255,0,0)<<16|RGB(255,255,0); // red FG, yellow BG
    palette[3]=RGB(0,180,0)<<16|RGB(0,0,128); // green on blue
    // now a little gradient
    for (int i=0;i<16;i++) 
      palette[4+i] = RGB(0,255-i*15,i*15)<<16| RGB(0,0,128);
    palette[20]=RGB(255,255,255)<<16|RGB(0,0,128);


    // make a window with attribute 1 (orange/yellow)
    text_color=1;
    window(0,0,17,17);
    print_at(5,0,"[Ascii]");

    //  draw ascii set with attribute 2 (red/yellow)
    for (int i=0;i<256;i++) 
    {
        vram[1+i/16][1+i%16]=i;
        vram_attr[1+i/16][1+i%16]=2;
    }
    
    text_color=20;
    window(23,1,64,10);
    //print_at(37,1,"[savestate info]");
    //print_at(24,2,a_game_buffer[0]);
    //print_at(24,3,a_game_buffer[1]);
    //sprintf(a_game_buffer[0], "  was at game %u", current_a_game);
    //print_at(24,4,a_game_buffer[0]);
    //sprintf(a_game_buffer[0], "  was attempting %u", attempting_a_game);
    //print_at(24,5,a_game_buffer[0]);

    text_color=0;
    print_at(31,SCREEN_H-14, "Sorry Bitbox...");
    for (int i=0;i<4;i++) 
    {
      text_color=4+i*4;
      print_at(31+4*i,SCREEN_H-13,"####");
    }

    // print text with a gradient 
    const char *bsod_text = "You just encountered the blue screen of death.";
    for (int i=0;i<10;i++) 
    {
      text_color=4+i;
      print_at(16-i,20+i,bsod_text);
    }

    cursor.attr = 2;
    place_cursor(0, SCREEN_H-1);
}
