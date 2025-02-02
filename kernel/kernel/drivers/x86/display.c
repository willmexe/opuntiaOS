#include <drivers/x86/display.h>
#include <drivers/x86/uart.h>

// prints

void ext_print_char(char symbol, unsigned char color, int col, int row, int minus_offset)
{
    char* mem = VIDEO_MEMORY;
    unsigned short offset_types[2] = { get_cursor_offet(), get_offset(col, row) };
    unsigned short offset = offset_types[(col_in_field(col) && row_in_field(row))] - minus_offset;
    if (symbol == '\n') {
        offset += MAX_COLS - (offset % MAX_COLS) - 1; // -1 to make it works (offset + 1 further)
    } else {
        mem[offset * 2] = symbol;
        mem[offset * 2 + 1] = color;
    }
    if (offset + 1 >= MAX_COLS * MAX_ROWS) {
        scroll(1); // scroll for 1 line
    } else {
        set_cursor_offset(offset + 1);
    }
}

void delete_char(unsigned char color, int col, int row, int minus_offset)
{
    char* mem = VIDEO_MEMORY;
    unsigned short offset_types[2] = { get_cursor_offet(), get_offset(col, row) };
    unsigned short offset = offset_types[(col_in_field(col) && row_in_field(row))] - minus_offset;
    mem[offset * 2] = ' ';
    mem[offset * 2 + 1] = color;
    set_cursor_offset(offset);
}

void print_char(char symbol, unsigned char color, int col, int row)
{
    char* mem = VIDEO_MEMORY;
    unsigned short offset_types[2] = { get_cursor_offet(), get_offset(col, row) };
    unsigned short offset = offset_types[(col_in_field(col) && row_in_field(row))];
    if (symbol == '\n') {
        offset += MAX_COLS - (offset % MAX_COLS) - 1; // -1 to make it works (offset + 1 further)
    } else {
        mem[offset * 2] = symbol;
        mem[offset * 2 + 1] = color;
    }
    if (offset + 1 >= MAX_COLS * MAX_ROWS) {
        scroll(1); // scroll for 1 line
    } else {
        set_cursor_offset(offset + 1);
    }
}

void print_string(const char* string, unsigned char color, int col, int row)
{
    int i = 0;
    while (string[i] != 0) {
        print_char(string[i++], color, col, row);
        col = row = -1;
    }
}

void print_hex(uint32_t hex, unsigned char color, int col, int row)
{
    uint32_t pk = (uint32_t)0x10000000;
    bool was_not_zero = 0;

    if (hex == 0) {
        print_char('0', color, col, row);
    } else {
        print_char('0', color, col, row);
        print_char('x', color, col, row);
    }

    while (pk > 0) {
        uint32_t pp = hex / pk;
        if (was_not_zero || pp > 0) {
            if (pp >= 10) {
                print_char(pp - 10 + 'A', color, col, row);
            } else {
                print_char(pp + '0', color, col, row);
            }
            was_not_zero = 1;
        }
        hex -= pp * pk;
        pk /= 16;
    }
}

void print_dec(uint32_t dec, unsigned char color, int col, int row)
{
    uint32_t pk = 1000000000;
    bool was_not_zero = 0;
    while (pk > 0) {
        uint32_t pp = dec / pk;
        if (was_not_zero || pp > 0) {
            print_char(pp + '0', color, col, row);
            was_not_zero = 1;
        }
        dec -= pp * pk;
        pk /= 10;
    }
    if (!was_not_zero) {
        print_char('0', color, col, row);
    }
}

void kprintf(const char* format, ...)
{
    char* ptr_to_next_arg = (char*)((uint32_t)&format + sizeof(void*));
    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == 'x') {
                uint32_t* arg = (uint32_t*)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(uint32_t);
                kprinth(*arg);
            } else if (*format == 'd') {
                uint32_t* arg = (uint32_t*)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(uint32_t);
                kprintd(*arg);
            } else if (*format == 's') {
                char** arg = (char**)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(char**);
                print_string(*arg, WHITE_ON_BLACK, -1, -1);
            }
        } else {
            print_char(*format, WHITE_ON_BLACK, -1, -1);
        }
        format++;
    }
    // print_string(string, WHITE_ON_BLACK, -1, -1);
}

void kprinth(uint32_t hex)
{
    print_hex(hex, WHITE_ON_BLACK, -1, -1);
}

void kprintd(uint32_t dec)
{
    print_dec(dec, WHITE_ON_BLACK, -1, -1);
}

// clean screen

void clean_screen()
{
    char* mem = VIDEO_MEMORY;
    for (int i = 0; i < 2 * MAX_COLS * MAX_ROWS; i += 2) {
        mem[i] = ' ';
        mem[i + 1] = WHITE_ON_BLACK;
    }
    set_cursor_offset(0);
}

// scroll

void scroll(unsigned char lines)
{
    char* mem = VIDEO_MEMORY;
    unsigned int mem_offset = (unsigned int)lines * MAX_COLS;
    unsigned int bytes_2_del = 2 * (MAX_COLS * MAX_ROWS - mem_offset);
    char* src = (mem + mem_offset * 2);
    memcpy(mem, src, bytes_2_del);

    for (unsigned int i = bytes_2_del; i < 2 * MAX_COLS * MAX_ROWS; i += 2) {
        mem[i] = ' ';
        mem[i + 1] = WHITE_ON_BLACK;
    }

    set_cursor_offset(bytes_2_del / 2);
}

// cursor

void set_cursor_offset(unsigned short offset)
{
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
}

int get_cursor_offet()
{
    port_byte_out(REG_SCREEN_CTRL, 14);
    unsigned short offset = port_byte_in(REG_SCREEN_DATA);
    offset <<= 8;
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset;
}

// helpers

int get_offset(int col, int row) { return row * MAX_COLS + col; }
char col_in_field(int col) { return (0 <= col && col <= MAX_COLS); }
char row_in_field(int row) { return (0 <= row && row <= MAX_ROWS); }
