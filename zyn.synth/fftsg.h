#ifndef FFTSG_H
#define FFTSG_H

void cdft(int n, int isgn, double *a, int *ip, double *w);
void rdft(int n, int isgn, double *a, int *ip, double *w);
void ddct(int n, int isgn, double *a, int *ip, double *w);
void ddst(int n, int isgn, double *a, int *ip, double *w);
void dfct(int n, double *a, double *t, int *ip, double *w);
void dfst(int n, double *a, double *t, int *ip, double *w);

#endif // FFTSG_H