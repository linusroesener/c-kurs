
typedef struct Gof Gof;

typedef enum {
  GOF_LIFE, GOF_DEAD
} GofState;

Gof *gof_create(int, int);
void gof_destroy(Gof *);

int gof_width(Gof *);
int gof_height(Gof *);

void gof_set(Gof *, int, int, GofState);
GofState gof_get(Gof *, int, int);
void gof_step(Gof *);

void gof_print(Gof *);




