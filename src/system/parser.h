#ifndef _DARK_COMMAND_H_
#define _DARK_COMMAND_H_

#include "header.h"
#include <Dark/inout>

namespace dark {

/**
 * @brief Command parser.
 * Aligned to 1024 Bytes.
 * 
 */
class command_parser {
  private:
    char buffer[924];  /* Buffered string. */
    int  index [24];   /* Index of string. */
    command_t command; /* Temporary command holder. */

    /**
     * @brief Get command from buffer.
     * 
     * @return True  If command requires further parasing 
     *      || False If command requires no more parasing,
     */
    bool get_command() noexcept {
        if(buffer[0] == 'q') {
            if     (buffer[9] == 'f') command = command_t::Q_PR;
            else if(buffer[9] == 'k') command = command_t::Q_TK;
            else if(buffer[9] == 'e') command = command_t::Q_OR;
            else if(buffer[9] == 'i') command = command_t::Q_TR;
            else /* buffer[9] == 'f'*/command = command_t::Q_TF;
        } else if(buffer[0] == 'b') {
            command = command_t::B_TK;
        } else if(buffer[0] == 'l') {  
            command = (buffer[4] == 'n' ? command_t::L_IN : command_t::L_OU); 
        } else if(buffer[0] == 'm') {
            command = command_t::M_PR;            
        } else if(buffer[0] == 'a') {
            command = (buffer[4] == 'u' ? command_t::A_US : command_t::A_TR);
        } else if(buffer[0] == 'r') {
            command = (buffer[4] == 'n' ? command_t::R_TK : command_t::R_TR);
        } else if(buffer[0] == 'd') {
            command = command_t::D_TR;
        } else return false;
        return true;
    }

  public:

    /**
     * @brief Parse the input line of string.
     * 
     * @return command_t Input command.
     */
    command_t parse() noexcept {
        /* First read and write out timestamp. */
        write_input();

        /* Then , get command. */
        read_string(buffer);
        if(!get_command()) /* EXIT or CLEAN contains no argument. */
            return buffer[0] == 'c' ? command_t::CLR_ : command_t::EXIT;
        /* If requires further parsing , first clear index.  */
        memset(index,-1,sizeof(index));
        char *head = buffer; /* The head index for reading in. */

        while(true) {
            char opt = read_string(head)[-1];
            index[opt - 'b'] = head - buffer;
            head = read_string(head);
            if(*head == '\n') opt = 0;
            *(head++) = 0; /* Null as end. */
            if(!opt) return command;
        }
    }

    /**
     * @brief Return the argument string. 
     * 
     * @param c Argument to find.
     * @return nullptr only if not in param.
     */
    char *get_argument(char c) noexcept
    { return index[c - 'b'] != -1 ? buffer + index[c - 'a'] : nullptr; }

};


}

#endif
