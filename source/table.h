#ifndef _TABLE_H_
#define _TABLE_H_


// Cátodo Común: bit0=a (top), bit1=b (top-right), bit2=c (bot-right),
//               bit3=d (bot), bit4=e (bot-left), bit5=f (top-left), bit6=g (middle)
//               bit7=dp (decimal point, si el hardware lo soporta)
//
//   aaa
//  f   b
//  f   b
//   ggg
//  e   c
//  e   c
//   ddd  [dp]

const unsigned char tabla7Seg[128] = {

    // --- ASCII 0-31: caracteres de control → apagado ---
    [0 ... 31] = 0x00,

    // --- ESPECIALES ---
    [' ']  = 0x00, //          apagado
    ['!']  = 0x06, // !        igual a '1'
    ['"']  = 0x22, // "        segmentos f, b
    ['#']  = 0x00, // #        sin representación
    ['$']  = 0x6D, // $        igual a 'S'
    ['%']  = 0x00, // %        sin representación
    ['&']  = 0x00, // &        sin representación
    ['\''] = 0x20, // '        segmento f (top-left)
    ['(']  = 0x39, // (        igual a 'C'
    [')']  = 0x0F, // )        a,b,c,d
    ['*']  = 0x00, // *        sin representación
    ['+']  = 0x00, // +        sin representación
    [',']  = 0x10, // ,        segmento f (bottom, informal)
    ['-']  = 0x40, // -        segmento g (middle bar)
    ['.']  = 0x80, // .        decimal point (bit7)
    ['/']  = 0x52, // /        b, e, g
    ['\\'] = 0x64, // \        f, g, c
    ['[']  = 0x39, // [        igual a 'C'
    [']']  = 0x0F, // ]        a, b, c, d
    ['=']  = 0x48, // =        d, g
    ['_']  = 0x08, // _        segmento d (bottom)
    ['?']  = 0x43, // ?        a, b, g (aproximado)
    ['@']  = 0x5F, // @        igual a 'a' minúscula
    ['`']  = 0x20, // `        segmento f
    ['^']  = 0x23, // ^        a, b, f

    // --- NÚMEROS ---
    ['0']  = 0x3F, //  _
                   // | |
                   // |_|
    ['1']  = 0x06, //   |
                   //   |
    ['2']  = 0x5B, //  _
                   //  _|
                   // |_
    ['3']  = 0x4F, //  _
                   //  _|
                   //  _|
    ['4']  = 0x66, // |_|
                   //   |
    ['5']  = 0x6D, //  _
                   // |_
                   //  _|
    ['6']  = 0x7D, //  _
                   // |_
                   // |_|
    ['7']  = 0x07, //  _
                   //   |
                   //   |
    ['8']  = 0x7F, //  _
                   // |_|
                   // |_|
    ['9']  = 0x6F, //  _
                   // |_|
                   //  _|

    // --- LETRAS MAYÚSCULAS ---
    ['A']  = 0x77, // a,b,c,e,f,g
    ['B']  = 0x7C, // igual a 'b' minúscula: c,d,e,f,g
    ['C']  = 0x39, // a,d,e,f
    ['D']  = 0x5E, // igual a 'd' minúscula: b,c,d,e,g
    ['E']  = 0x79, // a,d,e,f,g
    ['F']  = 0x71, // a,e,f,g
    ['G']  = 0x3D, // a,c,d,e,f
    ['H']  = 0x76, // b,c,e,f,g
    ['I']  = 0x06, // b,c
    ['J']  = 0x1E, // b,c,d,e
    ['K']  = 0x76, // aproximado como H
    ['L']  = 0x38, // d,e,f
    ['M']  = 0x00, // sin representación aceptable
    ['N']  = 0x37, // a,b,c,e,f (aproximado)
    ['O']  = 0x3F, // igual a '0'
    ['P']  = 0x73, // a,b,e,f,g
    ['Q']  = 0x67, // a,b,c,f,g
    ['R']  = 0x50, // igual a 'r' minúscula: e,g
    ['S']  = 0x6D, // igual a '5'
    ['T']  = 0x78, // igual a 't' minúscula: d,e,f,g
    ['U']  = 0x3E, // b,c,d,e,f
    ['V']  = 0x1C, // igual a 'u' minúscula: c,d,e
    ['W']  = 0x3E, // aproximado como U
    ['X']  = 0x76, // aproximado como H
    ['Y']  = 0x6E, // b,c,d,f,g
    ['Z']  = 0x5B, // igual a '2'

    // --- LETRAS MINÚSCULAS ---
    ['a']  = 0x5F, // a,b,c,d,e,g
    ['b']  = 0x7C, // c,d,e,f,g
    ['c']  = 0x58, // d,e,g
    ['d']  = 0x5E, // b,c,d,e,g
    ['e']  = 0x79, // a,d,e,f,g
    ['f']  = 0x71, // a,e,f,g
    ['g']  = 0x6F, // igual a '9'
    ['h']  = 0x74, // c,e,f,g
    ['i']  = 0x04, // segmento c
    ['j']  = 0x0E, // b,c,d
    ['k']  = 0x76, // aproximado como H
    ['l']  = 0x38, // d,e,f
    ['m']  = 0x00, // sin representación aceptable
    ['n']  = 0x54, // c,e,g
    ['o']  = 0x5C, // c,d,e,g
    ['p']  = 0x73, // a,b,e,f,g
    ['q']  = 0x67, // a,b,c,f,g
    ['r']  = 0x50, // e,g
    ['s']  = 0x6D, // igual a '5'
    ['t']  = 0x78, // d,e,f,g
    ['u']  = 0x1C, // c,d,e
    ['v']  = 0x1C, // igual a 'u'
    ['w']  = 0x3E, // aproximado como U
    ['x']  = 0x76, // aproximado como H
    ['y']  = 0x6E, // b,c,d,f,g
    ['z']  = 0x5B, // igual a '2'

    // --- ASCII 127: DEL → apagado ---
    [127]  = 0x00,
};


#endif
