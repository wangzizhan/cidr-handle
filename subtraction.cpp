#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <vector>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <map>
#include <stdbool.h>
#include "netmask.h"
using namespace std;

#define SIZE 40

typedef struct {
	string data;
} IPSET;

vector<IPSET> container_src;
vector<IPSET>::iterator src_iter;
vector<IPSET> container_patch;
vector<IPSET>::iterator patch_iter;
vector<IPSET> container_result;

map<uint32_t, uint32_t> range;
map<uint32_t, uint32_t>::iterator range_iter;

int main(int argc, char *argv[])
{
	uint32_t ip_start_src, ip_end_src, ip_start_patch, ip_end_patch;
	FILE *fp_src, *fp_patch;
	char ipset_src_row[SIZE];
	char ipset_patch_row[SIZE];
	int i, j, len;

	IPSET ipset_src;
	IPSET ipset_patch;

	fp_src = fopen(argv[1], "r");
	fp_patch = fopen(argv[2], "r");

	/* push to vector */		
	while (fgets(ipset_src_row, SIZE, fp_src) != NULL) {
		len = strlen(ipset_src_row);
		if (ipset_src_row[len - 1] == '\n') { 
			ipset_src_row[len - 1] = '\0';	
			ipset_src.data = ipset_src_row;
		}
		container_src.push_back(ipset_src);	
	} 

	while (fgets(ipset_patch_row, SIZE, fp_patch) != NULL) {
		if (!isalpha(ipset_patch_row[0])) {
			len = strlen(ipset_patch_row);
			if (ipset_patch_row[len - 1] == '\n') { 	
				ipset_patch_row[len - 1] = '\0';
				ipset_patch.data = ipset_patch_row;
			}
			container_patch.push_back(ipset_patch);
		}
	}

	/* filter */
	bool is_src = false;
	for (src_iter = container_src.begin(); src_iter != container_src.end(); ++src_iter) {
		for (patch_iter = container_patch.begin(); patch_iter != container_patch.end(); ++patch_iter) {
			if ((*src_iter).data == (*patch_iter).data) {
				is_src = false;
				break;
			} else {
				is_src = true;	
			}
		}			
		if (is_src) {
			container_result.push_back(*src_iter);
		}
	}
	
	/* compare */
	char *net, *mask, *cidr;
	int mask_int;
	uint32_t begin_src, end_src, begin_patch, end_patch;
	uint32_t begin_result_left = 0;
	uint32_t end_result_left = 0;
	uint32_t begin_result_right = 0;
	uint32_t end_result_right = 0;

	uint32_t start;
	uint32_t end;
	for (i = 0; i < (int)container_result.size(); i++) {
		if (isalpha(container_result[i].data[0])) {
			cout << container_result[i].data << endl;
		} else {
			string tmp = container_src[i].data;
			char *tmp_c = new char[tmp.length() + 1];
			strcpy(tmp_c, tmp.c_str());

			cidr = strtok(tmp_c, " - ");
			net = strtok(cidr, "/");
			mask = strtok(NULL, "/");
			mask_int = atoi(mask);

			begin_src = ntohl(inet_addr(net));
			end_src = begin_src + (1 << (32 - mask_int)) - 1;

		}
		for(j = 0; j < (int)container_patch.size(); j++) {
			if (isalpha(container_result[i].data[0])) {
				break;
			}
			string tmp = container_patch[j].data;
			char *tmp_c = new char[tmp.length() + 1];
			strcpy(tmp_c, tmp.c_str());

			cidr = strtok(tmp_c, " - ");
			net = strtok(cidr, "/");
			mask = strtok(NULL, "/");
			mask_int = atoi(mask);
			begin_patch = ntohl(inet_addr(net));
			end_patch = begin_patch + (1 << (32 - mask_int)) - 1;	

			/* first time we use source data */		
			if (j == 0) {
				/* begin_patch == end_patch && begin_src == end_src (3) */
				if (begin_patch == end_patch && begin_src == end_src && end_patch < begin_src) {
					begin_result_left = begin_src;
					end_result_left = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				}
				else if (begin_patch == end_patch && begin_src == end_src && end_patch == begin_src) {
					break;
				}	
				else if (begin_patch == end_patch && begin_src == end_src && begin_patch > end_src) {
					begin_result_left = begin_src;
					end_result_left = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				}
				/* begin_patch < end_patch && begin_src == end_src (5) */
				else if (begin_patch < end_patch && begin_src == end_src && end_patch < begin_src) {
					begin_result_left = begin_src;
					end_result_left = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				} 
				else if (begin_patch < end_patch && begin_src == end_src && end_patch == begin_src) {
					break;
				}
				else if (begin_patch < end_patch && begin_src == end_src && begin_patch < begin_src && end_src < end_patch) {
					break;
				}
				else if (begin_patch < end_patch && begin_src == end_src && begin_patch == begin_src) {
					break;
				}
				else if (begin_patch < end_patch && begin_src == end_src && begin_patch > end_src) {
					begin_result_left = begin_src;
					end_result_left = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				}
				/* begin_patch == end_patch && begin_src < end_src (5) */
				else if (begin_patch == end_patch && begin_src < end_src && end_patch < begin_src) {
					begin_result_left = begin_src;
					end_result_left = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				}
				else if (begin_patch == end_patch && begin_src < end_src && begin_src == end_patch) {
					begin_result_left = begin_src + 1;
					end_result_left = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				}
				else if (begin_patch == end_patch && begin_src < end_src && begin_patch > begin_src && end_patch < end_src) {
					begin_result_left = begin_src;
					end_result_left = begin_patch - 1;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));

					begin_result_right = end_patch + 1;
					end_result_right = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_right, end_result_right));
				}
				else if (begin_patch == end_patch && begin_src < end_src && end_patch == end_src) {
					begin_result_left = begin_src;
					end_result_left = end_src - 1;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				}
				else if (begin_patch == end_patch && begin_src < end_src && end_src < begin_patch) {
					begin_result_left = begin_src;
					end_result_left = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));

				}
				/* begin_patch < end_patch && begin_src < end_src (9) */	
				else if (begin_patch < end_patch && begin_src < end_src && end_patch < begin_src) {
					begin_result_left = begin_src;
					end_result_left = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				}
				else if (begin_patch < end_patch && begin_src < end_src && end_patch == begin_src) {
					begin_result_left = begin_src;		
					end_result_left = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				}
				else if (begin_patch < end_patch && begin_src < end_src && end_patch < end_src && begin_patch < begin_src && end_patch > begin_src) {
					begin_result_left = begin_src;
					end_result_left = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				} 
				else if (begin_patch < end_patch && begin_src < end_src && end_patch < end_src && begin_patch == begin_src) {
					begin_result_left = end_patch + 1;
					end_result_left = end_src;	
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				}
				else if (begin_patch < end_patch && begin_src < end_src && end_patch < end_src && begin_patch > begin_src) {
					begin_result_left = begin_src;
					end_result_left = begin_patch - 1;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));

					begin_result_right = end_patch + 1;
					end_result_right = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_right, end_result_right));
				}
				else if (begin_patch < end_patch && begin_src < end_src && end_patch == end_src && begin_patch > begin_src) {
					begin_result_left = begin_src;
					end_result_left = begin_patch - 1;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				}
				else if (begin_patch < end_patch && begin_src < end_src && end_patch > end_src && begin_patch > begin_src && end_src > begin_patch) {
					begin_result_left = begin_src;
					end_result_left = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				}
				else if (begin_patch < end_patch && begin_src < end_src && begin_patch == end_src) {
					begin_result_left = begin_src;
					end_result_left = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				}
				else if (begin_patch < end_patch && begin_src < end_src && begin_patch > end_src) {
					begin_result_left = begin_src;
					end_result_left = end_src;
					range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
				}
				/* others (1) */	
				else if (begin_patch < end_patch && begin_src < end_src && begin_patch <= begin_src && end_patch >= end_src) {
					break;
				}
			}
			/* now we use map data */ 
			else {
				for (range_iter = range.begin(); range_iter != range.end(); range_iter++) {
					begin_src = range_iter->first;
					end_src = range_iter->second;
					/* begin_patch == end_patch && begin_src == end_src (3) */
					if (begin_patch == end_patch && begin_src == end_src && end_patch < begin_src) {
						continue;
					}
					else if (begin_patch == end_patch && begin_src == end_src && end_patch == begin_src) {
						range.erase(begin_src);
					}	
					else if (begin_patch == end_patch && begin_src == end_src && begin_patch > end_src) {
						continue;
					}
					/* begin_patch < end_patch && begin_src == end_src (5) */
					else if (begin_patch < end_patch && begin_src == end_src && end_patch < begin_src) {
						continue;
					} 
					else if (begin_patch < end_patch && begin_src == end_src && end_patch == begin_src) {
						range.erase(begin_src);
					}
					else if (begin_patch < end_patch && begin_src == end_src && begin_patch < begin_src && end_src < end_patch) {
						range.erase(begin_src);
					}
					else if (begin_patch < end_patch && begin_src == end_src && begin_patch == begin_src) {
						range.erase(begin_src);
					}
					else if (begin_patch < end_patch && begin_src == end_src && begin_patch > end_src) {
						continue;
					}
					/* begin_patch == end_patch && begin_src < end_src (5) */
					else if (begin_patch == end_patch && begin_src < end_src && end_patch < begin_src) {
						continue;
					}
					else if (begin_patch == end_patch && begin_src < end_src && begin_src == end_patch) {
						range.erase(begin_src);

						begin_result_left = begin_src + 1;
						end_result_left = end_src;
						range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
					}
					else if (begin_patch == end_patch && begin_src < end_src && begin_patch > begin_src && end_patch < end_src) {
						range.erase(begin_src);

						begin_result_left = begin_src;
						end_result_left = begin_patch - 1;
						range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));

						begin_result_right = end_patch + 1;
						end_result_right = end_src;
						range.insert(pair<uint32_t, uint32_t>(begin_result_right, end_result_right));
					}
					else if (begin_patch == end_patch && begin_src < end_src && end_patch == end_src) {
						range.erase(begin_src);

						begin_result_left = begin_src;
						end_result_left = end_src - 1;
						range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
					}
					else if (begin_patch == end_patch && begin_src < end_src && end_src < begin_patch) {
						continue;
					}
					/* begin_patch < end_patch && begin_src < end_src (9) */	
					else if (begin_patch < end_patch && begin_src < end_src && end_patch < begin_src) {
						continue;
					}
					else if (begin_patch < end_patch && begin_src < end_src && end_patch == begin_src) {
						continue;
					}
					else if (begin_patch < end_patch && begin_src < end_src && end_patch < end_src && begin_patch < begin_src && end_patch > begin_src) {
						continue;
					} 
					else if (begin_patch < end_patch && begin_src < end_src && end_patch < end_src && begin_patch == begin_src) {
						range.erase(begin_src);

						begin_result_left = end_patch + 1;
						end_result_left = end_src;	
						range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
					}
					else if (begin_patch < end_patch && begin_src < end_src && end_patch < end_src && begin_patch > begin_src) {
						range.erase(begin_src);

						begin_result_left = begin_src;
						end_result_left = begin_patch - 1;
						range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));

						begin_result_right = end_patch + 1;
						end_result_right = end_src;
						range.insert(pair<uint32_t, uint32_t>(begin_result_right, end_result_right));
					}
					else if (begin_patch < end_patch && begin_src < end_src && end_patch == end_src && begin_patch > begin_src) {
						range.erase(begin_src);

						begin_result_left = begin_src;
						end_result_left = begin_patch - 1;
						range.insert(pair<uint32_t, uint32_t>(begin_result_left, end_result_left));
					}
					else if (begin_patch < end_patch && begin_src < end_src && end_patch > end_src && begin_patch > begin_src && end_src > begin_patch) {
						continue;
					}
					else if (begin_patch < end_patch && begin_src < end_src && begin_patch == end_src) {
						continue;
					}
					else if (begin_patch < end_patch && begin_src < end_src && begin_patch > end_src) {
						continue;
					}
					/* others (1) */	
					else if (begin_patch < end_patch && begin_src < end_src && begin_patch <= begin_src && end_patch >= end_src) {
						range.erase(begin_src);
					}	
				}
			}
		}

		for (range_iter = range.begin(); range_iter != range.end(); range_iter++) {
			start = range_iter->first;
			end = range_iter->second;
			transfer(start, end);	
			range.erase(range_iter);
		}	 
	}
	return 0;
}
