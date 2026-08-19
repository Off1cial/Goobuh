#ifndef EVENT_H
#define EVENT_H

typedef enum eventclass_t
{
  EVENT_QUIT,
  EVENT_WIN_RESIZE,
  EVENT_WIN_MOVE,
  EVENT_WIN_LOSEFOCUS,
  EVENT_WIN_GAINFOCUS,
} eventclass_t;

typedef struct event_t
{
  eventclass_t type;
  union 
  {
    struct 
    {
      int width, height;
    } win_resize;
  };
  
} event_t;

#endif