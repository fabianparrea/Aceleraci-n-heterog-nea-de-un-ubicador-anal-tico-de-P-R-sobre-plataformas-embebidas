#ifndef DCT_H
#define DCT_H

// DCT-II 2D en el arreglo (nx*ny, fila iy*nx+ix), en su lugar.
void dct2d_forward(float *data, int nx, int ny);
// Inversa exacta de dct2d_forward.
void dct2d_inverse(float *data, int nx, int ny);

#endif
