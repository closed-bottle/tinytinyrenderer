#ifndef TINYTINYRENDERER_COMMANDBUFF_H
#define TINYTINYRENDERER_COMMANDBUFF_H
#include "special-lamp/lampList.h++"




class CommandBuff {
    friend class RenderCmd;
    bool is_active_ = false;
    bool is_rendering_ = false;

    Lamp::list<int> excution_list_;
};

#endif //TINYTINYRENDERER_COMMANDBUFF_H