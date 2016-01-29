#include "screen.h"
#include "simple.h"
#include "common.h"

void screen_frame()
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
            }
            break;

//        case evt_keyboard_release: 
//            print_at(31,18,"KB released     ");
//            break;

//        case evt_device_change:
//            // It seems the disconnect event is not sent currently...
//            if (e.device.type == device_unconnected)
//                print_at(31, 18, "dev. unconnected");
//            else if (e.device.type == device_keyboard)
//                print_at(31, 18, "keyboard found! ");
//            break;

//        case evt_user:
//            print_at(31, 18, "user event      ");
//            break;
//        
//        default:
//            print_at(31, 18, "UNHANDLED       ");
        }
        e=event_get();
    }
}

void screen_init()
{
    // init palette
    palette[0]=RGB(240,240,240)<<16 | RGB(0,0,0);
    palette[1]=RGB(255,255,255)<<16 | RGB(0,0,0);
    palette[2]=RGB(0,0,0)<<16 | RGB(255,0,0);

    cursor.attr = 2;
    place_cursor(0, SCREEN_H-1);
}
