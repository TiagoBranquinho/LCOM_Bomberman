#include "game.h"
#include "libs.h"
#include "bomberman.xpm"

extern uint8_t bb[2];
extern bool two_byte;
extern int kbd_error;
extern uint32_t no_interrupts;

int(mainLoop)(){  
  sprite_t *player = sprite_constructor((const char* const*)bomberman_xpm);
  sprite_set_pos(player, 10, 10);
  sprite_draw(player);
  int ipc_status, r;
  uint8_t keyboard_sel;
  message msg;  
  bool make;
  if(kbd_subscribe_int(&keyboard_sel))
    return 1;
  int kbc_irq_set = BIT(keyboard_sel);

  uint8_t timer_sel;
  no_interrupts = 0;

  if(timer_subscribe_int(&timer_sel))
    return 1;
  int timer_irq_set = BIT(timer_sel);

  int process = 1;

  while( process ) { /* You may want to use a different condition */
     /* Get a request message. */
     if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
         printf("driver_receive failed with: %d", r);
         continue;
     }
     if (is_ipc_notify(ipc_status)) { /* received notification */
         switch (_ENDPOINT_P(msg.m_source)) {
             case HARDWARE: /* hardware interrupt notification */       
                 if (msg.m_notify.interrupts & kbc_irq_set) { /* subscribed interrupt */
                      kbc_ih();/* process it */
                      if(two_byte || kbd_error){
                        continue;
                      }
                      keyboard_get_code(&make, bb);
                      process = !keyboard_process_key(bb, player);

                 }
                 if (msg.m_notify.interrupts & timer_irq_set) { /* subscribed interrupt */
                     timer_int_handler();   /* process it */
                     if((no_interrupts * 60) % REFRESH_RATE == 0){ // atualiza a cada 1 segundo
                        vg_clear_screen();
                        sprite_draw(player);
                 }
                 }
                 break;
             default:
                 break; /* no other notifications expected: do nothing */ 
         }
     } else { /* received a standard message, not a notification */
         /* no standard messages expected: do nothing */
     }
  }

  kbd_unsubscribe_int();
  vg_exit();
  sprite_destructor(player);
  return OK;
  
}

