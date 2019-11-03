#ifndef WAGASCI_MAPPING_H_
#define WAGASCI_MAPPING_H_

struct map_cell {
  int plane;
  int channel;
  float x;
  float y;
  float z;
} mapcell;

// Read the filename mapping file and populate the map_cell struct with the
// channel mapping. nb_chips is the total number of ROC chips, nb_chans is the
// number of channels for every chip. dif is an integer uniquely identifying
// the DIF whose mapping file refers to.
struct map_cell **load_mapping(char *filename,int nb_chips,int nb_chans,int dif);
void free_mapping(struct map_cell **mapping,int nb_chips);

#endif // WAGASCI_MAPPING_H_
