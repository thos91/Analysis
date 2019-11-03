#include "libmapping.h"
#include <stdio.h>
#include <stdlib.h>

struct map_cell **load_mapping(char *filename,int nb_chips,int nb_chans,int dif) {

  struct map_cell **mapping;
  FILE *f;
  int i;
  int chip,chan;
  int plane,channel;
  float x,y,z;
  int res;
  int dif_id;

  //allocate mapping
  mapping=malloc(sizeof(struct map_cell *)*nb_chips);
  for (i=0;i<nb_chips;i++)
    mapping[i]=malloc(sizeof(struct map_cell)*nb_chans);

  //open mapping file
  f=fopen(filename,"r");
  if (f==NULL) {
    printf("cant open file %s\n",filename);
    return NULL;
  }

  //reading mapping
  while(!feof(f)) {
    res=fscanf(f,"%d %d %d %f %f %f %d %d",&dif_id,&chip,&chan,&x,&y,&z,&plane,&channel);
    if ((res!=8)&&(res!=-1)) {
      printf("syntax error in file %s obtained %d items instead of 8\n",filename,res);
    } else {
      // printf("dif_id=%d chip=%d chan=%d x=%f y=%f z=%f pln=%d ch=%d\n",dif_id,chip,chan,x,y,z,plane,channel);
      }
    if (dif==dif_id) {
      mapping[chip][chan].x=x;
      mapping[chip][chan].y=y;
      mapping[chip][chan].z=z;
      mapping[chip][chan].plane=plane;
      mapping[chip][chan].channel=channel;
    }
  }
  fclose(f);
  return mapping;
}

void free_mapping(struct map_cell **mapping,int nb_chips) {
  int i;
  for (i=0;i<nb_chips;i++)
    free(mapping[i]);
  free(mapping);
}
