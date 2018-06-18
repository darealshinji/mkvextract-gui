/*
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */

#ifndef ROTATE_H_INCLUDED
#define ROTATE_H_INCLUDED

/* png files were created from rotate.svg */

unsigned char __1_png[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18,
  0x08, 0x06, 0x00, 0x00, 0x00, 0xe0, 0x77, 0x3d, 0xf8, 0x00, 0x00, 0x00,
  0x04, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0x08, 0x7c, 0x08, 0x64,
  0x88, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x00,
  0x59, 0x00, 0x00, 0x00, 0x59, 0x01, 0xaa, 0x9d, 0xa9, 0x86, 0x00, 0x00,
  0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61,
  0x72, 0x65, 0x00, 0x77, 0x77, 0x77, 0x2e, 0x69, 0x6e, 0x6b, 0x73, 0x63,
  0x61, 0x70, 0x65, 0x2e, 0x6f, 0x72, 0x67, 0x9b, 0xee, 0x3c, 0x1a, 0x00,
  0x00, 0x01, 0xc0, 0x49, 0x44, 0x41, 0x54, 0x48, 0x89, 0xc5, 0x96, 0x31,
  0x6e, 0x13, 0x41, 0x18, 0x85, 0xdf, 0x6f, 0x28, 0x50, 0x68, 0xd3, 0x78,
  0xcc, 0x2d, 0xf6, 0x12, 0x14, 0x33, 0x27, 0x48, 0x1b, 0x8f, 0xd6, 0x5a,
  0xc9, 0x01, 0x94, 0x03, 0xfc, 0x3e, 0x01, 0x89, 0x88, 0x83, 0x6c, 0x87,
  0x32, 0x07, 0x88, 0x67, 0x0a, 0x1a, 0x0a, 0x8a, 0x74, 0xb6, 0x38, 0x04,
  0xd8, 0x6e, 0xd2, 0x46, 0x4a, 0xc5, 0xa3, 0x59, 0x07, 0xc5, 0xda, 0xdd,
  0xec, 0x00, 0x11, 0x4f, 0x9a, 0x66, 0xfe, 0xfd, 0xdf, 0xb7, 0xb3, 0x3b,
  0x6f, 0x34, 0x40, 0x9a, 0xbe, 0x94, 0xa3, 0xb5, 0x9e, 0x27, 0x02, 0x5e,
  0x24, 0x3e, 0x8f, 0x4e, 0x6a, 0xc3, 0x5f, 0x01, 0xe6, 0xf3, 0xf9, 0x71,
  0x08, 0xe1, 0x84, 0xa4, 0xfc, 0x81, 0x97, 0xf4, 0x7a, 0xbd, 0x13, 0x63,
  0xcc, 0x71, 0x2d, 0x40, 0x44, 0x5e, 0x01, 0x38, 0x8a, 0x31, 0x8e, 0x13,
  0x21, 0x62, 0x8c, 0x19, 0x93, 0x3c, 0x2a, 0x3d, 0xaa, 0x01, 0xd6, 0xda,
  0xb7, 0x00, 0xce, 0x00, 0x0c, 0x62, 0x8c, 0x53, 0x55, 0x6d, 0xf3, 0x09,
  0xc5, 0x18, 0xf3, 0x01, 0xc0, 0x00, 0xc0, 0x6c, 0xb5, 0x5a, 0xbd, 0x6b,
  0x5a, 0x01, 0xad, 0xb5, 0xc3, 0x12, 0x72, 0x98, 0x65, 0xd9, 0xe4, 0x11,
  0xc8, 0xd6, 0xbc, 0x00, 0x30, 0x5b, 0xaf, 0xd7, 0x39, 0x80, 0x9f, 0xb5,
  0x80, 0x16, 0x90, 0x9b, 0x72, 0xb4, 0x32, 0x6f, 0x14, 0x49, 0x09, 0x21,
  0x9c, 0x87, 0x10, 0x18, 0x42, 0xe8, 0xef, 0xd6, 0xbb, 0xdd, 0x6e, 0xdf,
  0x18, 0x43, 0x63, 0xcc, 0x39, 0x80, 0xda, 0xff, 0x55, 0x9b, 0x03, 0x11,
  0x21, 0xc9, 0x22, 0xc6, 0xf8, 0x0d, 0xc0, 0x65, 0x45, 0xfd, 0x92, 0x24,
  0x36, 0x9b, 0xcd, 0x05, 0x00, 0xb6, 0x7e, 0xf3, 0x7f, 0x2d, 0x99, 0x4c,
  0x26, 0x5f, 0x01, 0xbc, 0xac, 0x2a, 0x92, 0xbc, 0xc9, 0xf3, 0xfc, 0x75,
  0x93, 0x41, 0x96, 0x65, 0x9f, 0x01, 0xec, 0x57, 0x9a, 0x8b, 0xdc, 0x3e,
  0x79, 0x92, 0xff, 0xbb, 0x64, 0x34, 0x1a, 0xf5, 0x55, 0x75, 0x6f, 0xb7,
  0xe0, 0x9c, 0xdb, 0xb3, 0xd6, 0xf6, 0xd1, 0xb0, 0x83, 0x80, 0xe6, 0xc3,
  0x4e, 0x54, 0x75, 0x4c, 0x72, 0x2a, 0x22, 0x07, 0xbb, 0x45, 0x92, 0x07,
  0x22, 0x32, 0x75, 0xce, 0x8d, 0x9b, 0x20, 0x75, 0x00, 0x51, 0xd5, 0xfb,
  0xf8, 0x93, 0xfc, 0x04, 0x00, 0xde, 0xfb, 0x2b, 0xef, 0xfd, 0x15, 0x00,
  0xc4, 0x18, 0x2f, 0x48, 0x9e, 0x01, 0x18, 0x38, 0xe7, 0x6a, 0x8f, 0x95,
  0xaa, 0xc9, 0xad, 0x79, 0x01, 0x60, 0x06, 0x20, 0x57, 0xd5, 0x6d, 0x42,
  0xf7, 0xf1, 0x7b, 0xc7, 0x30, 0xc6, 0x38, 0x2c, 0x21, 0x87, 0xcb, 0xe5,
  0xb2, 0xf2, 0x58, 0xd9, 0x9d, 0x68, 0x32, 0xaf, 0xd2, 0xa3, 0x90, 0x07,
  0x49, 0x56, 0xd5, 0xf7, 0xa5, 0xf9, 0x47, 0x55, 0x2d, 0xd0, 0x2e, 0xa1,
  0x8c, 0x31, 0x0e, 0x9d, 0x73, 0xcf, 0x00, 0x0c, 0x16, 0x8b, 0xc5, 0x2d,
  0x80, 0x37, 0x75, 0x2b, 0xf8, 0x01, 0xe0, 0x34, 0xc1, 0xfc, 0x1e, 0x12,
  0x42, 0x28, 0x48, 0x9e, 0x76, 0x3a, 0x9d, 0xef, 0x09, 0x7d, 0x0f, 0xe5,
  0xbd, 0xbf, 0xf6, 0xde, 0x5f, 0xa7, 0xf4, 0x3c, 0x79, 0x92, 0x93, 0x6e,
  0x15, 0x22, 0x72, 0x97, 0x0a, 0xf8, 0x05, 0x5e, 0xad, 0xcd, 0x6b, 0xf9,
  0x01, 0xbe, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae,
  0x42, 0x60, 0x82
};
unsigned int __1_png_len = 579;

unsigned char __2_png[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18,
  0x08, 0x06, 0x00, 0x00, 0x00, 0xe0, 0x77, 0x3d, 0xf8, 0x00, 0x00, 0x00,
  0x04, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0x08, 0x7c, 0x08, 0x64,
  0x88, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x00,
  0x59, 0x00, 0x00, 0x00, 0x59, 0x01, 0xaa, 0x9d, 0xa9, 0x86, 0x00, 0x00,
  0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61,
  0x72, 0x65, 0x00, 0x77, 0x77, 0x77, 0x2e, 0x69, 0x6e, 0x6b, 0x73, 0x63,
  0x61, 0x70, 0x65, 0x2e, 0x6f, 0x72, 0x67, 0x9b, 0xee, 0x3c, 0x1a, 0x00,
  0x00, 0x01, 0xb8, 0x49, 0x44, 0x41, 0x54, 0x48, 0x89, 0xc5, 0x96, 0x3d,
  0x4e, 0xdc, 0x40, 0x18, 0x86, 0x9f, 0x2f, 0x49, 0x11, 0x91, 0x96, 0xc6,
  0xbb, 0x9c, 0x62, 0xe7, 0x12, 0x29, 0xe6, 0x06, 0xb4, 0x78, 0xcc, 0xca,
  0xd2, 0x92, 0x44, 0x1c, 0x60, 0x6e, 0x10, 0x10, 0x2c, 0xd9, 0x19, 0x93,
  0x32, 0x07, 0x40, 0x6e, 0xd2, 0x50, 0xa4, 0xa0, 0xb4, 0x73, 0x88, 0xa4,
  0xa4, 0x45, 0x4a, 0xe5, 0x8f, 0x22, 0xde, 0x44, 0xac, 0x6c, 0xb3, 0x4e,
  0x84, 0x78, 0x25, 0x37, 0xdf, 0xcf, 0xfb, 0xc8, 0xa3, 0x79, 0x2d, 0xc3,
  0x08, 0x95, 0x65, 0x79, 0x5d, 0x96, 0xe5, 0xf5, 0x98, 0x9d, 0x57, 0x63,
  0x86, 0x81, 0xd7, 0x23, 0xe7, 0x79, 0x31, 0x76, 0xe1, 0xbf, 0x00, 0x31,
  0xc6, 0xe3, 0xd5, 0x6a, 0x75, 0xa2, 0xaa, 0xf2, 0x0f, 0x5e, 0x02, 0x9c,
  0x00, 0xc7, 0xbd, 0x80, 0xa6, 0x69, 0xf6, 0x44, 0xe4, 0x28, 0xc6, 0xb8,
  0x1c, 0x09, 0x11, 0x60, 0x09, 0x1c, 0x01, 0x7b, 0xbd, 0x80, 0x2c, 0xcb,
  0xde, 0x03, 0xe7, 0xc0, 0x3c, 0xc6, 0x18, 0xbd, 0xf7, 0xdb, 0x1c, 0xa1,
  0x00, 0x67, 0xc0, 0x1c, 0x28, 0x80, 0x0f, 0xbd, 0x00, 0x11, 0x51, 0xe7,
  0xdc, 0xa2, 0x85, 0x1c, 0x24, 0x49, 0x12, 0x1e, 0x81, 0xac, 0xcd, 0xf3,
  0xd6, 0xfc, 0x10, 0x68, 0x7a, 0x01, 0x5b, 0x40, 0x6e, 0xdb, 0x67, 0x2b,
  0xf3, 0x41, 0xa9, 0xaa, 0x84, 0x10, 0x2e, 0x42, 0x08, 0x1a, 0x63, 0x4c,
  0x3b, 0x46, 0x52, 0x40, 0x81, 0x8b, 0x16, 0xd6, 0xa9, 0xde, 0x1c, 0x88,
  0x88, 0xaa, 0x6a, 0x5e, 0x14, 0xc5, 0x77, 0x55, 0xfd, 0xd2, 0x31, 0xb2,
  0xae, 0x5d, 0xb6, 0xa0, 0xe7, 0x91, 0x78, 0xef, 0xbf, 0x01, 0x6f, 0x7a,
  0xfa, 0xb7, 0xde, 0xfb, 0xb7, 0x43, 0x06, 0x93, 0xc9, 0xe4, 0x2b, 0xb0,
  0xdb, 0xd3, 0xbe, 0x7b, 0xf2, 0x24, 0x3f, 0xbb, 0x24, 0x4d, 0xd3, 0xd4,
  0x39, 0xb7, 0xb3, 0xd9, 0x30, 0xc6, 0xec, 0xcc, 0x66, 0xb3, 0x94, 0x81,
  0x1b, 0x04, 0xc3, 0x1f, 0x3b, 0x71, 0xce, 0x2d, 0x45, 0x24, 0xaa, 0xea,
  0xfe, 0x66, 0x53, 0x55, 0xf7, 0x45, 0x24, 0x1a, 0x63, 0x96, 0x43, 0x90,
  0x3e, 0x80, 0x38, 0xe7, 0xce, 0x80, 0xb9, 0x88, 0x14, 0xd3, 0xe9, 0xf4,
  0x33, 0x80, 0xb5, 0xf6, 0xca, 0x5a, 0x7b, 0x05, 0x50, 0xd7, 0xf5, 0xa5,
  0xaa, 0x9e, 0x03, 0x73, 0x63, 0x4c, 0xec, 0xf3, 0xea, 0x2a, 0xae, 0xcd,
  0x73, 0x11, 0x29, 0x92, 0x24, 0x39, 0xf4, 0xde, 0xaf, 0x13, 0xba, 0xcb,
  0xdf, 0x1b, 0xa3, 0x75, 0x5d, 0x2f, 0x5a, 0xc8, 0x81, 0x31, 0x26, 0x74,
  0xf9, 0x6d, 0x16, 0x86, 0xcc, 0xbb, 0xf4, 0x28, 0xe4, 0x41, 0x92, 0xb3,
  0x2c, 0xfb, 0xa8, 0xaa, 0x39, 0xf0, 0x29, 0x84, 0x90, 0xb3, 0x5d, 0x42,
  0xb5, 0xae, 0xeb, 0x85, 0x31, 0xe6, 0x25, 0xbf, 0x8f, 0xeb, 0xae, 0xaa,
  0xaa, 0x77, 0x9d, 0x6f, 0xd0, 0x34, 0xcd, 0x4f, 0x11, 0x39, 0x8d, 0x31,
  0x6e, 0x6b, 0xfe, 0x07, 0x52, 0x55, 0x55, 0x0e, 0x9c, 0x02, 0x3f, 0x46,
  0xec, 0x3d, 0x94, 0xb5, 0xf6, 0xc6, 0x5a, 0x7b, 0x33, 0x66, 0xe7, 0xc9,
  0x93, 0x3c, 0xea, 0xaf, 0x42, 0x44, 0x7e, 0x8d, 0x05, 0xdc, 0x03, 0xbd,
  0xbf, 0xb5, 0x50, 0x69, 0xa3, 0xa1, 0x07, 0x00, 0x00, 0x00, 0x00, 0x49,
  0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};
unsigned int __2_png_len = 571;

unsigned char __3_png[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18,
  0x08, 0x06, 0x00, 0x00, 0x00, 0xe0, 0x77, 0x3d, 0xf8, 0x00, 0x00, 0x00,
  0x04, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0x08, 0x7c, 0x08, 0x64,
  0x88, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x00,
  0x59, 0x00, 0x00, 0x00, 0x59, 0x01, 0xaa, 0x9d, 0xa9, 0x86, 0x00, 0x00,
  0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61,
  0x72, 0x65, 0x00, 0x77, 0x77, 0x77, 0x2e, 0x69, 0x6e, 0x6b, 0x73, 0x63,
  0x61, 0x70, 0x65, 0x2e, 0x6f, 0x72, 0x67, 0x9b, 0xee, 0x3c, 0x1a, 0x00,
  0x00, 0x01, 0xc4, 0x49, 0x44, 0x41, 0x54, 0x48, 0x89, 0xc5, 0x96, 0xbd,
  0x6e, 0xd4, 0x40, 0x14, 0x85, 0xcf, 0x01, 0x0a, 0x14, 0xda, 0x34, 0x3b,
  0xce, 0x53, 0x8c, 0xf6, 0x21, 0x28, 0x6c, 0xbf, 0x40, 0x5a, 0xd6, 0xc2,
  0xb2, 0x94, 0x10, 0x94, 0x7e, 0xef, 0x1b, 0x10, 0xb4, 0xac, 0x62, 0x7b,
  0xa1, 0xcc, 0x03, 0x64, 0x3d, 0x05, 0x4d, 0x0a, 0x8a, 0x94, 0xfb, 0x14,
  0xb0, 0xde, 0x26, 0x6d, 0x24, 0x44, 0xc1, 0x4d, 0x63, 0x85, 0xcd, 0xca,
  0x3f, 0x19, 0xa1, 0xc0, 0x95, 0xdc, 0xcc, 0x99, 0x7b, 0x3e, 0xf9, 0xda,
  0x67, 0x6c, 0xc0, 0xa3, 0xf2, 0x3c, 0xbf, 0xca, 0xf3, 0xfc, 0xca, 0xa7,
  0xe7, 0x85, 0xcf, 0x66, 0x92, 0x2f, 0x7d, 0xf6, 0x03, 0xc0, 0x33, 0xdf,
  0x86, 0xbf, 0x02, 0x88, 0xc8, 0xa9, 0x88, 0x9c, 0x01, 0xa0, 0xaf, 0x91,
  0xaa, 0xb2, 0xaa, 0xaa, 0xb3, 0xe5, 0x72, 0x79, 0xda, 0x09, 0x00, 0x70,
  0x00, 0xe0, 0x58, 0x44, 0xe6, 0x3e, 0x10, 0x55, 0xa5, 0x73, 0x6e, 0x0e,
  0xe0, 0x98, 0xe4, 0x41, 0x27, 0x40, 0x44, 0x4e, 0x00, 0x7c, 0x02, 0x90,
  0x8a, 0x48, 0x29, 0x22, 0x83, 0x23, 0x6c, 0xcc, 0x67, 0x00, 0x52, 0x00,
  0x8b, 0xd5, 0x6a, 0xf5, 0xbe, 0xef, 0x0e, 0x54, 0x44, 0x8e, 0x1a, 0xc8,
  0x1b, 0x00, 0x45, 0x1f, 0x64, 0xcb, 0x3c, 0x6b, 0xcc, 0xdf, 0x8a, 0xc8,
  0xef, 0x3e, 0xc0, 0x10, 0xe4, 0xa6, 0xb9, 0x1e, 0x65, 0x0e, 0xf4, 0xcf,
  0x99, 0xcd, 0xb3, 0x48, 0x49, 0x26, 0xd3, 0xe9, 0x74, 0xb1, 0x2d, 0x56,
  0x55, 0x35, 0x01, 0x50, 0x02, 0x38, 0x0f, 0xc3, 0x30, 0x23, 0xa9, 0x6d,
  0x26, 0x7d, 0x33, 0x56, 0x11, 0xc9, 0x48, 0x26, 0xaa, 0x7a, 0xd1, 0xa2,
  0x5f, 0x00, 0x48, 0xfa, 0xcc, 0xff, 0x49, 0x31, 0x49, 0x92, 0x6f, 0x24,
  0x5f, 0x75, 0xe8, 0x37, 0x45, 0x51, 0xbc, 0x1e, 0xf0, 0xf8, 0x0a, 0x60,
  0xbf, 0x43, 0xbb, 0x7d, 0xf2, 0x24, 0xff, 0xf7, 0x62, 0x18, 0x86, 0x93,
  0x28, 0x8a, 0xf6, 0x76, 0x05, 0x63, 0xcc, 0xde, 0x68, 0x34, 0x9a, 0x60,
  0x20, 0xf1, 0x7d, 0x23, 0x62, 0x14, 0x45, 0x73, 0x92, 0xa5, 0xaa, 0x1e,
  0xee, 0x8a, 0xaa, 0x7a, 0x48, 0xb2, 0x34, 0xc6, 0xf4, 0x1e, 0x2b, 0x5d,
  0x00, 0xc6, 0x71, 0x3c, 0x03, 0x90, 0xaa, 0xea, 0x62, 0x3c, 0x1e, 0x7f,
  0x01, 0x00, 0x6b, 0xed, 0xa5, 0xb5, 0xf6, 0x12, 0x00, 0x36, 0x9b, 0xcd,
  0x67, 0x34, 0xc7, 0x8a, 0x31, 0xa6, 0xec, 0xf2, 0x6a, 0x5b, 0x64, 0x1c,
  0xc7, 0x33, 0x55, 0xcd, 0x1a, 0xf3, 0xed, 0x84, 0xee, 0xe3, 0xcf, 0x1b,
  0xa3, 0x75, 0x5d, 0xdf, 0x27, 0xde, 0x18, 0x53, 0xb4, 0xf9, 0xed, 0x2e,
  0xf4, 0x99, 0xb7, 0xd5, 0x20, 0xe4, 0xc1, 0x17, 0x2d, 0x0c, 0xc3, 0x0f,
  0xaa, 0x9a, 0x01, 0x38, 0x77, 0xce, 0x65, 0xce, 0xb9, 0xc7, 0x24, 0x54,
  0xeb, 0xba, 0x3e, 0x32, 0xc6, 0x3c, 0x07, 0x90, 0x06, 0x41, 0x70, 0xbb,
  0x5e, 0xaf, 0xdf, 0xb5, 0x02, 0x48, 0xfe, 0x50, 0xd5, 0x8f, 0xce, 0xb9,
  0x13, 0x00, 0x3e, 0xf1, 0xd7, 0xba, 0xae, 0xb3, 0x20, 0x08, 0x7e, 0x01,
  0xf8, 0xee, 0xd1, 0xf7, 0xb0, 0xac, 0xb5, 0xd7, 0xd6, 0xda, 0x6b, 0x9f,
  0x9e, 0x27, 0x4f, 0xb2, 0xef, 0x5f, 0xc5, 0x4f, 0x5f, 0xc0, 0x1d, 0x53,
  0x96, 0xd2, 0xd7, 0x53, 0x89, 0xef, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x49,
  0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};
unsigned int __3_png_len = 583;

unsigned char __4_png[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18,
  0x08, 0x06, 0x00, 0x00, 0x00, 0xe0, 0x77, 0x3d, 0xf8, 0x00, 0x00, 0x00,
  0x04, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0x08, 0x7c, 0x08, 0x64,
  0x88, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x00,
  0x59, 0x00, 0x00, 0x00, 0x59, 0x01, 0xaa, 0x9d, 0xa9, 0x86, 0x00, 0x00,
  0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61,
  0x72, 0x65, 0x00, 0x77, 0x77, 0x77, 0x2e, 0x69, 0x6e, 0x6b, 0x73, 0x63,
  0x61, 0x70, 0x65, 0x2e, 0x6f, 0x72, 0x67, 0x9b, 0xee, 0x3c, 0x1a, 0x00,
  0x00, 0x01, 0xb3, 0x49, 0x44, 0x41, 0x54, 0x48, 0x89, 0xc5, 0x96, 0x3d,
  0x6e, 0xdb, 0x40, 0x10, 0x46, 0xdf, 0x28, 0x29, 0x02, 0xa7, 0x4d, 0xb5,
  0xf0, 0x29, 0xb4, 0x3c, 0x44, 0x1a, 0x9d, 0xc0, 0x6d, 0xb8, 0xb2, 0xc0,
  0xc0, 0x8e, 0x03, 0x1f, 0x60, 0x6f, 0x10, 0x07, 0xb2, 0x60, 0x69, 0x58,
  0xfb, 0x00, 0x96, 0x9a, 0x34, 0x2e, 0x52, 0xb8, 0xe4, 0x29, 0x12, 0x08,
  0x2a, 0xdc, 0x1a, 0x08, 0x52, 0x68, 0xd2, 0x50, 0x81, 0x8c, 0xf0, 0xc7,
  0x8b, 0xc0, 0xc9, 0x07, 0x10, 0x24, 0x30, 0x33, 0xdf, 0x5b, 0x0e, 0x39,
  0x5c, 0x42, 0x82, 0x62, 0x8c, 0xb7, 0x31, 0xc6, 0xdb, 0x94, 0x9a, 0x97,
  0x29, 0xc9, 0xc0, 0xab, 0xc4, 0x7c, 0x06, 0xa9, 0x05, 0x7f, 0x05, 0xc8,
  0xf3, 0xfc, 0x7c, 0x3c, 0x1e, 0x5f, 0x00, 0x92, 0x6a, 0x64, 0x66, 0x32,
  0x9f, 0xcf, 0x2f, 0x54, 0xf5, 0xbc, 0x15, 0x30, 0x18, 0x0c, 0x0e, 0xcd,
  0xec, 0x34, 0x84, 0x30, 0x4b, 0x81, 0x98, 0x99, 0xa8, 0xea, 0x4c, 0x44,
  0x4e, 0xb7, 0xdb, 0xed, 0x61, 0x2b, 0x60, 0xb1, 0x58, 0x9c, 0x01, 0x97,
  0xc0, 0x24, 0x84, 0xa0, 0x31, 0xc6, 0xde, 0x16, 0xd6, 0xe6, 0x53, 0x60,
  0x62, 0x66, 0xe5, 0x66, 0xb3, 0xf9, 0xd8, 0x0a, 0x00, 0x4c, 0x55, 0x4f,
  0x6a, 0xc8, 0xbb, 0xf5, 0x7a, 0xbd, 0xe8, 0x82, 0xec, 0x99, 0x17, 0xb5,
  0xf9, 0x71, 0x8c, 0x71, 0xdb, 0x05, 0xe8, 0x83, 0xdc, 0xd7, 0xc7, 0x93,
  0xcc, 0xa1, 0xbb, 0xcf, 0x52, 0x3f, 0x8b, 0x89, 0x99, 0x85, 0xb2, 0x2c,
  0xcb, 0xfd, 0xa0, 0xaa, 0xe6, 0x66, 0xa6, 0xc0, 0x55, 0x08, 0xa1, 0x10,
  0x11, 0x6b, 0x32, 0xe9, 0xea, 0xb1, 0xa9, 0x6a, 0x61, 0x66, 0x41, 0x44,
  0xae, 0xff, 0x08, 0x9a, 0x5d, 0x8b, 0x48, 0xe8, 0x32, 0xff, 0x27, 0x92,
  0xd1, 0x68, 0xf4, 0x55, 0x44, 0x5e, 0xb7, 0xc4, 0xef, 0x97, 0xcb, 0xe5,
  0xdb, 0x2e, 0x83, 0xd5, 0x6a, 0xf5, 0x05, 0x78, 0xd3, 0x14, 0x33, 0xb3,
  0x87, 0x67, 0x9f, 0xe4, 0xff, 0x2e, 0x19, 0x0e, 0x87, 0xb9, 0xf7, 0xfe,
  0xa0, 0x21, 0x76, 0x00, 0xe4, 0xf4, 0x4c, 0x7c, 0x57, 0x8b, 0xc4, 0x7b,
  0x3f, 0x13, 0x11, 0x35, 0xb3, 0xa3, 0x86, 0xf8, 0x11, 0xa0, 0x40, 0xd2,
  0x67, 0xe5, 0xb7, 0x79, 0x96, 0x65, 0x97, 0xde, 0x7b, 0xf3, 0xde, 0xeb,
  0x6e, 0x21, 0xce, 0xb9, 0x1b, 0xe7, 0xdc, 0xcd, 0x2e, 0x07, 0x98, 0x02,
  0x06, 0x94, 0x6d, 0x8b, 0x6d, 0xda, 0x0f, 0x24, 0xcb, 0xb2, 0xa9, 0x99,
  0x15, 0x40, 0x59, 0x55, 0xd5, 0x31, 0xb0, 0x9b, 0xd0, 0xfd, 0xb7, 0xc5,
  0x80, 0x93, 0xfa, 0xfa, 0x7d, 0x7d, 0x1e, 0xef, 0xe5, 0x36, 0x02, 0xba,
  0xcc, 0x9b, 0xd4, 0x0b, 0x79, 0x04, 0xf0, 0xde, 0x7f, 0xaa, 0xcd, 0xaf,
  0xaa, 0xaa, 0x2a, 0x6a, 0x83, 0x3e, 0xed, 0x20, 0x2f, 0x80, 0x09, 0xf0,
  0x00, 0x7c, 0x68, 0xbb, 0x83, 0xef, 0xc0, 0xe7, 0xaa, 0xaa, 0xce, 0x9e,
  0x68, 0xbe, 0x0f, 0x29, 0x80, 0x9f, 0xc0, 0xb7, 0x84, 0xba, 0xc7, 0x72,
  0xce, 0xdd, 0x39, 0xe7, 0xee, 0x52, 0x6a, 0x9e, 0x7d, 0x92, 0x53, 0xff,
  0x2a, 0x7e, 0xa4, 0x02, 0x7e, 0x01, 0xbf, 0x61, 0xce, 0xff, 0x92, 0xec,
  0x6d, 0xad, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42,
  0x60, 0x82
};
unsigned int __4_png_len = 566;

unsigned char __5_png[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18,
  0x08, 0x06, 0x00, 0x00, 0x00, 0xe0, 0x77, 0x3d, 0xf8, 0x00, 0x00, 0x00,
  0x04, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0x08, 0x7c, 0x08, 0x64,
  0x88, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x00,
  0x59, 0x00, 0x00, 0x00, 0x59, 0x01, 0xaa, 0x9d, 0xa9, 0x86, 0x00, 0x00,
  0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61,
  0x72, 0x65, 0x00, 0x77, 0x77, 0x77, 0x2e, 0x69, 0x6e, 0x6b, 0x73, 0x63,
  0x61, 0x70, 0x65, 0x2e, 0x6f, 0x72, 0x67, 0x9b, 0xee, 0x3c, 0x1a, 0x00,
  0x00, 0x01, 0xb8, 0x49, 0x44, 0x41, 0x54, 0x48, 0x89, 0xc5, 0x96, 0x31,
  0x6e, 0xdb, 0x40, 0x10, 0x45, 0xff, 0x24, 0x29, 0x02, 0xa7, 0x75, 0xa3,
  0x95, 0x6f, 0x41, 0x1d, 0x22, 0x05, 0x79, 0x02, 0xb7, 0x96, 0x68, 0x81,
  0x80, 0x1c, 0x07, 0x3e, 0xc0, 0xe8, 0x04, 0x71, 0x60, 0x09, 0x56, 0xa8,
  0xda, 0x07, 0x08, 0x66, 0x8a, 0x34, 0x2e, 0x52, 0xa4, 0xe4, 0x29, 0x12,
  0xb1, 0x72, 0x6b, 0x20, 0x48, 0x91, 0xef, 0x86, 0x0e, 0x1c, 0x99, 0xa4,
  0xc8, 0x04, 0x4e, 0x06, 0x98, 0x66, 0xff, 0xee, 0x7f, 0xcb, 0x25, 0x3e,
  0xb9, 0x40, 0x8f, 0x4a, 0xd3, 0xf4, 0x3a, 0x4d, 0xd3, 0xeb, 0x3e, 0x6b,
  0x5e, 0xf4, 0x99, 0x4c, 0xf2, 0x65, 0x9f, 0xf9, 0x00, 0xf0, 0xac, 0xef,
  0x82, 0xbf, 0x02, 0x24, 0x49, 0x72, 0x16, 0xc7, 0xf1, 0x39, 0x00, 0xf9,
  0x03, 0x2f, 0x51, 0xd5, 0x73, 0x55, 0x3d, 0x6b, 0x04, 0x90, 0x3c, 0x10,
  0x91, 0x93, 0x24, 0x49, 0x96, 0x3d, 0x21, 0xa2, 0xaa, 0x4b, 0x00, 0x27,
  0x00, 0x0e, 0x1a, 0x01, 0xee, 0x7e, 0x4a, 0x72, 0x01, 0x60, 0x9a, 0x24,
  0x49, 0xae, 0xaa, 0x5d, 0x8e, 0x50, 0x54, 0xf5, 0x02, 0xc0, 0x14, 0xc0,
  0x1a, 0xc0, 0xdb, 0x46, 0x00, 0x00, 0xba, 0xfb, 0xac, 0x82, 0x1c, 0x15,
  0x45, 0xf1, 0x61, 0x07, 0xe4, 0xde, 0x3c, 0xab, 0xcc, 0x8f, 0x55, 0xf5,
  0x67, 0x1b, 0x60, 0x17, 0xe4, 0xa6, 0xea, 0x4e, 0xe6, 0x40, 0xfb, 0x39,
  0x4b, 0xf5, 0x2e, 0xa6, 0x24, 0x27, 0xee, 0xbe, 0x7e, 0x28, 0xce, 0xe7,
  0xf3, 0x31, 0xc9, 0x1c, 0xc0, 0xa5, 0xaa, 0x66, 0x00, 0x58, 0x67, 0xd2,
  0xf6, 0xf8, 0x34, 0xb3, 0x8c, 0xe4, 0x44, 0x44, 0xae, 0x1e, 0x89, 0xe4,
  0x95, 0x88, 0x4c, 0xda, 0xcc, 0xff, 0x49, 0xc9, 0x68, 0x34, 0xfa, 0x4c,
  0xf2, 0x55, 0x83, 0x7e, 0x53, 0x14, 0xc5, 0xeb, 0x36, 0x83, 0xd5, 0x6a,
  0xf5, 0x49, 0x44, 0xf6, 0x1b, 0xe4, 0xdb, 0x27, 0x4f, 0xf2, 0x7f, 0x2f,
  0x19, 0x0c, 0x06, 0xe3, 0x10, 0xc2, 0xde, 0xb6, 0x60, 0x66, 0x7b, 0x66,
  0x36, 0x26, 0xd9, 0x9a, 0xf8, 0xd6, 0x10, 0x85, 0x10, 0x96, 0x22, 0x92,
  0x93, 0x3c, 0xac, 0xd1, 0x0f, 0x01, 0xe4, 0xee, 0xbe, 0xdc, 0x05, 0x69,
  0x32, 0x5f, 0x84, 0x10, 0x18, 0x42, 0xc8, 0x1f, 0x6c, 0xe4, 0x63, 0xd5,
  0x20, 0x29, 0x66, 0x76, 0x61, 0x66, 0x34, 0xb3, 0x75, 0x53, 0xe2, 0xeb,
  0x06, 0x25, 0x84, 0xf0, 0x2b, 0xa1, 0x65, 0x59, 0x1e, 0x03, 0xb8, 0x4f,
  0xe8, 0x7e, 0xd5, 0x10, 0x11, 0xc6, 0x71, 0x3c, 0x03, 0xb0, 0x00, 0x70,
  0x14, 0x45, 0x51, 0xed, 0x67, 0x65, 0x7b, 0xa0, 0xcd, 0xfc, 0xf1, 0x4e,
  0x3a, 0x40, 0x7e, 0xfb, 0xa3, 0x0d, 0x87, 0xc3, 0x77, 0x24, 0x33, 0x00,
  0x97, 0x65, 0x59, 0x76, 0x4a, 0xa8, 0x88, 0x90, 0xe4, 0xcc, 0xdd, 0x9f,
  0x03, 0x98, 0x46, 0x51, 0x74, 0x0b, 0xe0, 0x4d, 0x2d, 0x80, 0xe4, 0x37,
  0x11, 0x79, 0xbf, 0xd9, 0x6c, 0x4e, 0xbb, 0x98, 0x6f, 0x41, 0x32, 0x77,
  0xff, 0x21, 0x22, 0x5f, 0xbb, 0xae, 0xab, 0xab, 0x2f, 0x55, 0x77, 0xae,
  0x27, 0x4f, 0x72, 0xaf, 0x5b, 0x05, 0x80, 0xef, 0x7d, 0x01, 0x77, 0x7b,
  0x15, 0xd6, 0xb8, 0x14, 0x08, 0x94, 0xa4, 0x00, 0x00, 0x00, 0x00, 0x49,
  0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};
unsigned int __5_png_len = 571;

unsigned char __6_png[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18,
  0x08, 0x06, 0x00, 0x00, 0x00, 0xe0, 0x77, 0x3d, 0xf8, 0x00, 0x00, 0x00,
  0x04, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0x08, 0x7c, 0x08, 0x64,
  0x88, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x00,
  0x59, 0x00, 0x00, 0x00, 0x59, 0x01, 0xaa, 0x9d, 0xa9, 0x86, 0x00, 0x00,
  0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61,
  0x72, 0x65, 0x00, 0x77, 0x77, 0x77, 0x2e, 0x69, 0x6e, 0x6b, 0x73, 0x63,
  0x61, 0x70, 0x65, 0x2e, 0x6f, 0x72, 0x67, 0x9b, 0xee, 0x3c, 0x1a, 0x00,
  0x00, 0x01, 0xb7, 0x49, 0x44, 0x41, 0x54, 0x48, 0x89, 0xc5, 0x96, 0x41,
  0x4e, 0xdb, 0x40, 0x14, 0x86, 0xbf, 0x97, 0x76, 0x51, 0xc1, 0x96, 0x45,
  0x1c, 0x71, 0x8a, 0xcc, 0x25, 0xd8, 0x58, 0x3d, 0x00, 0x5b, 0xec, 0x10,
  0xb9, 0x0a, 0xa5, 0xe2, 0x00, 0x73, 0x00, 0xa4, 0xd2, 0x42, 0x84, 0xfc,
  0xcc, 0x96, 0x03, 0x54, 0xd9, 0x74, 0xc3, 0xa2, 0x07, 0xb0, 0x7b, 0x88,
  0x56, 0xc9, 0x86, 0x2d, 0x52, 0xd5, 0x05, 0x8f, 0x8d, 0x91, 0x50, 0x62,
  0x4f, 0xe2, 0x22, 0xe8, 0x93, 0x46, 0x96, 0xe6, 0xcd, 0x7c, 0xdf, 0x8c,
  0xad, 0x5f, 0x32, 0x74, 0xa8, 0x38, 0x8e, 0x6f, 0xe2, 0x38, 0xbe, 0xe9,
  0xb2, 0xe7, 0x6d, 0x97, 0xc5, 0x66, 0xf6, 0xae, 0xcb, 0x7a, 0x80, 0x5e,
  0xd7, 0x0d, 0xcf, 0x12, 0x38, 0xe7, 0x4e, 0x9c, 0x73, 0x67, 0x80, 0xfc,
  0x03, 0x4b, 0x46, 0xa3, 0xd1, 0x59, 0x92, 0x24, 0x27, 0xad, 0x02, 0x60,
  0x17, 0x38, 0x72, 0xce, 0x4d, 0x3b, 0x4a, 0x24, 0x4d, 0xd3, 0xa9, 0x99,
  0x1d, 0xf5, 0x7a, 0xbd, 0xdd, 0x56, 0x41, 0x59, 0x96, 0xc7, 0x66, 0x76,
  0x01, 0x8c, 0x9d, 0x73, 0xda, 0x70, 0x80, 0x36, 0xf8, 0x39, 0x30, 0x16,
  0x91, 0x22, 0x8a, 0xa2, 0x4f, 0xa1, 0x1b, 0x58, 0x55, 0x55, 0x93, 0x5a,
  0x72, 0xe0, 0x9c, 0xcb, 0xd7, 0x48, 0x1e, 0xe1, 0x59, 0x0d, 0x3f, 0xf4,
  0xde, 0xdf, 0x87, 0x04, 0xeb, 0x24, 0xb7, 0xf5, 0xd8, 0x08, 0x0e, 0xe1,
  0xf7, 0x2c, 0xf5, 0xb7, 0x18, 0x9b, 0x59, 0x5a, 0x55, 0x55, 0xf1, 0xb4,
  0x99, 0x24, 0x49, 0x22, 0x22, 0x0a, 0x5c, 0xaa, 0x6a, 0x06, 0x58, 0x13,
  0x24, 0x94, 0x03, 0x2b, 0xcb, 0x32, 0x1b, 0x0e, 0x87, 0x3f, 0x45, 0xe4,
  0x7a, 0xc5, 0x2e, 0x72, 0x6d, 0x66, 0x14, 0x45, 0x71, 0xd5, 0x06, 0x7f,
  0x95, 0x92, 0xc1, 0x60, 0xf0, 0x03, 0xd8, 0x6e, 0xe9, 0xdf, 0xce, 0xe7,
  0xf3, 0xbd, 0x10, 0xc0, 0x7b, 0xff, 0x1d, 0xd8, 0x69, 0x69, 0xdf, 0xbd,
  0x78, 0x92, 0xff, 0x7b, 0x09, 0x90, 0x00, 0x5b, 0xcb, 0x8d, 0x3c, 0xcf,
  0xb7, 0x54, 0x35, 0x31, 0xb3, 0x60, 0xe2, 0x83, 0x21, 0x02, 0xa6, 0x80,
  0x02, 0xfb, 0x2b, 0x4d, 0x91, 0x7d, 0x33, 0x53, 0x55, 0x9d, 0x86, 0x24,
  0x6d, 0x0d, 0x01, 0xce, 0x81, 0x0c, 0x28, 0x80, 0x43, 0xe0, 0x7e, 0x36,
  0x9b, 0x7d, 0x03, 0x88, 0xe3, 0xf8, 0xbd, 0x99, 0x89, 0xaa, 0x7e, 0x05,
  0x3e, 0x00, 0x57, 0x8b, 0xc5, 0x62, 0xd4, 0x14, 0xb4, 0xa6, 0x1b, 0x34,
  0xc2, 0xeb, 0xde, 0x4e, 0x3d, 0x10, 0x11, 0x4b, 0xd3, 0x74, 0x02, 0x5c,
  0x00, 0x07, 0x51, 0x14, 0xe5, 0xde, 0xfb, 0x15, 0xde, 0xf2, 0x44, 0x08,
  0xbe, 0x7a, 0x92, 0x0d, 0x24, 0xcb, 0x49, 0xfe, 0x5c, 0xc3, 0x2f, 0xeb,
  0xe7, 0xda, 0x84, 0x8a, 0x88, 0x99, 0xd9, 0x44, 0x55, 0xdf, 0x00, 0xe3,
  0x7e, 0xbf, 0x7f, 0x07, 0x7c, 0x6c, 0x13, 0xfc, 0x06, 0xbe, 0x00, 0xc7,
  0x9b, 0xc0, 0x97, 0x24, 0x59, 0x9e, 0xe7, 0x7f, 0x45, 0xe4, 0x57, 0xe8,
  0x06, 0xa7, 0x9b, 0x42, 0x9b, 0x24, 0x3c, 0x39, 0xf9, 0x63, 0xbd, 0x78,
  0x92, 0x3b, 0xfd, 0x55, 0x00, 0x7f, 0xba, 0x0a, 0x1e, 0x00, 0xfc, 0xe8,
  0xb2, 0x86, 0x2f, 0x94, 0xc0, 0x16, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
  0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};
unsigned int __6_png_len = 570;

unsigned char __7_png[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18,
  0x08, 0x06, 0x00, 0x00, 0x00, 0xe0, 0x77, 0x3d, 0xf8, 0x00, 0x00, 0x00,
  0x04, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0x08, 0x7c, 0x08, 0x64,
  0x88, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x00,
  0x59, 0x00, 0x00, 0x00, 0x59, 0x01, 0xaa, 0x9d, 0xa9, 0x86, 0x00, 0x00,
  0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61,
  0x72, 0x65, 0x00, 0x77, 0x77, 0x77, 0x2e, 0x69, 0x6e, 0x6b, 0x73, 0x63,
  0x61, 0x70, 0x65, 0x2e, 0x6f, 0x72, 0x67, 0x9b, 0xee, 0x3c, 0x1a, 0x00,
  0x00, 0x01, 0xbc, 0x49, 0x44, 0x41, 0x54, 0x48, 0x89, 0xc5, 0x96, 0xbd,
  0x6e, 0xd4, 0x40, 0x14, 0x85, 0xcf, 0x45, 0x14, 0x28, 0xb4, 0x69, 0x7c,
  0x37, 0x6f, 0xe1, 0x7d, 0x08, 0x8a, 0x19, 0xbf, 0x40, 0x5a, 0x56, 0xcb,
  0xca, 0x12, 0x11, 0x28, 0xfd, 0xde, 0x7d, 0x01, 0x48, 0xb4, 0x6c, 0xa2,
  0xd9, 0x84, 0x32, 0x0f, 0x10, 0xd9, 0x05, 0x4d, 0x0a, 0x8a, 0x94, 0x2b,
  0x1e, 0x02, 0xd6, 0x55, 0x5a, 0x24, 0x2a, 0x0e, 0x8d, 0x81, 0xec, 0xc6,
  0x3f, 0x19, 0x20, 0xe2, 0x4a, 0x2e, 0x3c, 0x67, 0xee, 0xf9, 0x3c, 0xb6,
  0xcf, 0x68, 0x80, 0x88, 0x4a, 0xd3, 0xf4, 0x2a, 0x4d, 0xd3, 0xab, 0x98,
  0x9e, 0xc7, 0x31, 0x93, 0x49, 0x3e, 0x89, 0x99, 0x0f, 0x00, 0x8f, 0x62,
  0x1b, 0xfe, 0x0a, 0xa0, 0xaa, 0x87, 0x83, 0xc1, 0xe0, 0x08, 0x80, 0xfc,
  0x81, 0x97, 0x38, 0xe7, 0x8e, 0xbc, 0xf7, 0x87, 0xad, 0x00, 0x11, 0xd9,
  0x23, 0x79, 0xa0, 0xaa, 0x8b, 0x48, 0x88, 0x78, 0xef, 0x17, 0x22, 0x72,
  0x40, 0x72, 0xaf, 0x15, 0xb0, 0x5e, 0xaf, 0x5f, 0x01, 0x78, 0x07, 0x60,
  0xa2, 0xaa, 0xcb, 0x6d, 0xbd, 0xcd, 0x3c, 0xcb, 0xb2, 0x39, 0x80, 0x09,
  0xc9, 0xb3, 0xe1, 0x70, 0xf8, 0xba, 0x15, 0x00, 0x80, 0x55, 0x55, 0xbd,
  0xac, 0x21, 0xcf, 0x55, 0x35, 0xf4, 0x40, 0x24, 0xcb, 0xb2, 0x39, 0xc9,
  0xbc, 0x36, 0x7f, 0x61, 0x66, 0xdf, 0xbb, 0x00, 0x7d, 0x90, 0x9b, 0xfa,
  0xba, 0x97, 0x79, 0x5f, 0x89, 0xaa, 0x9e, 0xa8, 0x2a, 0x93, 0x24, 0x19,
  0x6d, 0x8b, 0xce, 0xb9, 0x91, 0xf7, 0x9e, 0xde, 0xfb, 0x13, 0x74, 0x7c,
  0xaf, 0xae, 0x1c, 0xb0, 0xaa, 0xaa, 0x3c, 0x49, 0x92, 0x4f, 0x22, 0x72,
  0x71, 0x87, 0x2e, 0x72, 0x41, 0x12, 0x65, 0x59, 0x9e, 0x03, 0x60, 0xcc,
  0x93, 0xff, 0xd3, 0x12, 0x00, 0x1f, 0x01, 0x3c, 0x6d, 0xd1, 0x6f, 0x00,
  0x3c, 0xeb, 0x32, 0x18, 0x8f, 0xc7, 0x1f, 0x00, 0xec, 0x36, 0x69, 0x24,
  0xbf, 0x3e, 0x78, 0x92, 0xff, 0x6f, 0x91, 0x94, 0xa2, 0x28, 0x46, 0x45,
  0x51, 0xec, 0x6c, 0x6b, 0x66, 0xb6, 0x33, 0x9b, 0xcd, 0x46, 0xe8, 0x49,
  0x7c, 0xeb, 0x2b, 0x22, 0x29, 0x65, 0x59, 0x2e, 0x00, 0x2c, 0x01, 0xec,
  0x6f, 0xeb, 0x22, 0xb2, 0x4f, 0x72, 0x69, 0x66, 0x9d, 0xdb, 0x4a, 0x23,
  0xa0, 0x36, 0x9f, 0x03, 0x98, 0x00, 0x38, 0x5b, 0xad, 0x56, 0xef, 0x01,
  0x20, 0x84, 0x70, 0x19, 0x42, 0xb8, 0x04, 0x80, 0xe9, 0x74, 0x7a, 0x8e,
  0x7a, 0x5b, 0x31, 0xb3, 0xa5, 0x99, 0x35, 0x7a, 0xdd, 0x19, 0xbc, 0x65,
  0x9e, 0xd7, 0xe6, 0xb7, 0x13, 0xba, 0x8b, 0xdf, 0x7f, 0x0c, 0xcd, 0xec,
  0x57, 0xe2, 0x01, 0x84, 0x26, 0xc8, 0xc6, 0x40, 0x8f, 0x79, 0xe3, 0x62,
  0xfb, 0x20, 0x1b, 0x37, 0x65, 0x59, 0xbe, 0xad, 0xcd, 0x4f, 0x9d, 0x73,
  0xe3, 0x7b, 0xee, 0x2d, 0x3f, 0x21, 0xa7, 0x35, 0xe4, 0x4d, 0xd7, 0x0a,
  0xbe, 0x00, 0x38, 0x76, 0xce, 0xe5, 0x22, 0x12, 0x13, 0x7f, 0x9a, 0x59,
  0x0e, 0xe0, 0x58, 0x44, 0x3e, 0x47, 0xf4, 0x6d, 0x56, 0x08, 0xe1, 0x3a,
  0x84, 0x70, 0x1d, 0xd3, 0xf3, 0xe0, 0x49, 0x8e, 0x3d, 0x55, 0x7c, 0x8b,
  0x05, 0xfc, 0x00, 0xcb, 0x6d, 0xd3, 0x5b, 0xbe, 0xaa, 0xc0, 0x1d, 0x00,
  0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};
unsigned int __7_png_len = 575;

unsigned char __8_png[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18,
  0x08, 0x06, 0x00, 0x00, 0x00, 0xe0, 0x77, 0x3d, 0xf8, 0x00, 0x00, 0x00,
  0x04, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0x08, 0x7c, 0x08, 0x64,
  0x88, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x00,
  0x59, 0x00, 0x00, 0x00, 0x59, 0x01, 0xaa, 0x9d, 0xa9, 0x86, 0x00, 0x00,
  0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61,
  0x72, 0x65, 0x00, 0x77, 0x77, 0x77, 0x2e, 0x69, 0x6e, 0x6b, 0x73, 0x63,
  0x61, 0x70, 0x65, 0x2e, 0x6f, 0x72, 0x67, 0x9b, 0xee, 0x3c, 0x1a, 0x00,
  0x00, 0x01, 0xbe, 0x49, 0x44, 0x41, 0x54, 0x48, 0x89, 0xb5, 0x96, 0xb1,
  0x6e, 0xd3, 0x40, 0x18, 0xc7, 0x7f, 0x5f, 0x02, 0x12, 0x0a, 0x6b, 0x27,
  0xa7, 0x4f, 0xe1, 0xcb, 0x43, 0x30, 0x24, 0x79, 0x81, 0x2e, 0x1d, 0xf0,
  0xa5, 0x91, 0xab, 0x16, 0x50, 0x1f, 0xe0, 0xde, 0x80, 0x56, 0x6d, 0x5a,
  0xe5, 0x0c, 0x0f, 0xd1, 0x28, 0x03, 0x4b, 0x07, 0x86, 0x8e, 0x09, 0x2b,
  0x3b, 0xc8, 0xc9, 0xd0, 0xb5, 0x12, 0x12, 0x12, 0x1f, 0x8b, 0x0b, 0x49,
  0xea, 0x3a, 0x5c, 0x4b, 0xbe, 0xc5, 0xd6, 0xfd, 0xef, 0xfb, 0xfd, 0x74,
  0xf6, 0x9d, 0x65, 0x08, 0xa8, 0x28, 0x8a, 0xae, 0xa2, 0x28, 0xba, 0x0a,
  0xe9, 0x79, 0x16, 0x32, 0x19, 0x78, 0x11, 0x38, 0x9f, 0x5a, 0x68, 0xc3,
  0x53, 0x05, 0x47, 0xc0, 0x31, 0x20, 0x8f, 0x60, 0x89, 0x31, 0xe6, 0xd8,
  0x18, 0x73, 0x54, 0x25, 0xd8, 0x06, 0x0e, 0x81, 0x41, 0xa0, 0x44, 0x8c,
  0x31, 0x83, 0xa2, 0x77, 0x7b, 0x31, 0x58, 0x7d, 0x07, 0x6f, 0x81, 0x3a,
  0xb0, 0x0f, 0x3c, 0x07, 0x7a, 0xc0, 0xaf, 0x75, 0xf0, 0x56, 0xab, 0x75,
  0xaa, 0xaa, 0x7d, 0x20, 0x9b, 0x4c, 0x26, 0xef, 0xaa, 0x04, 0x0a, 0x1c,
  0x14, 0xf7, 0xfb, 0xc5, 0xb5, 0x4a, 0x72, 0x07, 0x4f, 0x0b, 0xf8, 0xde,
  0xea, 0xdc, 0xb2, 0x5d, 0x54, 0x25, 0xb9, 0x09, 0x81, 0xaf, 0x2b, 0x01,
  0xce, 0x0b, 0x61, 0xb2, 0x1a, 0xc6, 0x71, 0x9c, 0x18, 0x63, 0xd4, 0x18,
  0x73, 0xce, 0xe3, 0x36, 0xc5, 0x1f, 0x49, 0x02, 0x34, 0x56, 0x03, 0x63,
  0x4c, 0x23, 0x8e, 0xe3, 0xe4, 0x29, 0xf0, 0xff, 0x52, 0x32, 0x1a, 0x8d,
  0x3e, 0x8b, 0xc8, 0xcb, 0x07, 0xf2, 0x9b, 0x4e, 0xa7, 0xf3, 0xaa, 0x0a,
  0xd0, 0xed, 0x76, 0x3f, 0x01, 0x5b, 0x65, 0x99, 0xaa, 0xde, 0x6e, 0xfc,
  0x24, 0x6f, 0xbc, 0xea, 0x55, 0xa1, 0xaa, 0x4a, 0xb3, 0xd9, 0x4c, 0xda,
  0xed, 0xf6, 0xd7, 0xf1, 0x78, 0xfc, 0x73, 0x31, 0xb3, 0xd6, 0x36, 0xe2,
  0x38, 0xde, 0x9d, 0x4e, 0xa7, 0x5f, 0xaa, 0x18, 0x0f, 0x3e, 0x22, 0x55,
  0x15, 0xef, 0xfd, 0x40, 0x55, 0xbd, 0x88, 0xec, 0x94, 0xe4, 0x3b, 0x22,
  0xe2, 0xad, 0xb5, 0x95, 0x9f, 0x95, 0x52, 0x41, 0x01, 0x3f, 0x05, 0xfa,
  0xaa, 0x9a, 0xe5, 0x79, 0xfe, 0x11, 0xc0, 0x39, 0x77, 0xe9, 0x9c, 0xbb,
  0x04, 0xc8, 0xb2, 0xec, 0x03, 0x70, 0x06, 0xf4, 0xad, 0xb5, 0xde, 0x39,
  0x57, 0xca, 0xba, 0x37, 0xb8, 0x00, 0x4f, 0x55, 0x35, 0x9b, 0xcf, 0xe7,
  0x7b, 0xce, 0xb9, 0xbb, 0x13, 0xba, 0xc5, 0xdf, 0x1d, 0xa3, 0xde, 0xfb,
  0x83, 0x42, 0xf2, 0x3a, 0xcf, 0xf3, 0x61, 0x99, 0x64, 0x69, 0x60, 0x0d,
  0xbc, 0x74, 0xb1, 0xeb, 0x24, 0x4b, 0xdf, 0xa2, 0xe1, 0x70, 0xf8, 0x5e,
  0x44, 0x52, 0xe0, 0xa2, 0xd7, 0xeb, 0xa5, 0x22, 0xa2, 0x15, 0xf0, 0x25,
  0x89, 0xb5, 0xb6, 0x0e, 0xf4, 0x67, 0xb3, 0xd9, 0x2d, 0xf0, 0xa6, 0x74,
  0x05, 0xb5, 0x5a, 0xed, 0xbb, 0xaa, 0x9e, 0x58, 0x6b, 0xff, 0x15, 0xbe,
  0x28, 0x49, 0x45, 0xe4, 0x44, 0x55, 0xbf, 0x05, 0xf4, 0x2d, 0x97, 0x73,
  0xee, 0xda, 0x39, 0x77, 0x1d, 0xd2, 0xb3, 0xf1, 0x93, 0x1c, 0xfa, 0x57,
  0xf1, 0x23, 0x54, 0xf0, 0x1b, 0xa1, 0x0c, 0xc6, 0xc6, 0x47, 0xf9, 0xca,
  0x7f, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60,
  0x82
};
unsigned int __8_png_len = 577;

#endif  /* ROTATE_H_INCLUDED */
