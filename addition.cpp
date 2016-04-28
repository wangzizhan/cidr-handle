#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <vector>
#include <iostream>
#include <stdint.h>
using namespace std;

#define SIZE 40

typedef struct {
	string data;
} IPSET;

typedef struct {
	string title;	
	int mark;
} KEY;

vector<IPSET> container_src;
vector<IPSET> container_patch;
vector<KEY> container_key_src;
vector<KEY> container_key_patch;

int main(int argc, char *argv[])
{
	uint32_t ip_start_src, ip_end_src, ip_start_patch, ip_end_patch;
	FILE *fp_src, *fp_patch;
	char ipset_src_row[SIZE];
	char ipset_patch_row[SIZE];
	int i, j;
	
	IPSET ipset_src;
	IPSET ipset_patch;
	KEY key_src;
	KEY key_patch;

	fp_src = fopen(argv[1], "r");
	fp_patch = fopen(argv[2], "r");

	/* push to vector */		
	while (fgets(ipset_src_row, SIZE, fp_src) != NULL) {
		ipset_src.data = ipset_src_row;
		container_src.push_back(ipset_src);	
	} 

	while (fgets(ipset_patch_row, SIZE, fp_patch) != NULL) {
		ipset_patch.data = ipset_patch_row;
		container_patch.push_back(ipset_patch);
	}

	/* compare */	
	for (i = 0; i < (int)container_src.size(); i++) {
		if (isalpha(container_src[i].data[0])) {
			key_src.title = container_src[i].data;
			key_src.mark = i;
			container_key_src.push_back(key_src);	
		}
	}

	for (j = 0; j < (int)container_patch.size(); j++) {
		if (isalpha(container_patch[j].data[0])) {
			key_patch.title = container_patch[j].data;
			key_patch.mark = j;
			container_key_patch.push_back(key_patch);
		}
	}

	int key_src_iter, key_patch_iter, src_iter, patch_iter;
	string titlesrc, titlepatch;
	int marksrc, markpatch;
 
	//for (m = 0; m < (int)container_key_src.size(); m++) { 
	//	cout << "key src title:" << container_key_src[m].title << "key src mark" << container_key_src[m].mark << endl;
	//} 
	//
	//for (n = 0; n < (int)container_key_patch.size(); n++) { 
	//	cout << "key patch title:" << container_key_patch[n].title << "key patch mark" << container_key_patch[n].mark << endl;
	//}

	for (key_src_iter = 0; key_src_iter < (int)container_key_src.size(); key_src_iter++) {

		titlesrc = container_key_src[key_src_iter].title;
		marksrc = container_key_src[key_src_iter].mark;

		//cout << "src " << titlesrc << marksrc << endl; 
		for (src_iter = marksrc; src_iter < (int)container_src.size(); src_iter++) {
			while (src_iter < container_key_src[key_src_iter + 1].mark) {
				cout << container_src[src_iter].data;
				break;
			}
			if (key_src_iter == (int)container_key_src.size() - 1) {
				cout << container_src[src_iter].data;
			}
		}
		//break;
		for (key_patch_iter = 0; key_patch_iter < (int)container_key_patch.size(); key_patch_iter++) {

			titlepatch = container_key_patch[key_patch_iter].title;
			markpatch = container_key_patch[key_patch_iter].mark;

			if (titlesrc == titlepatch) {
				for (patch_iter = markpatch; patch_iter < (int)container_patch.size(); patch_iter++) {
					while (patch_iter < container_key_patch[key_patch_iter + 1].mark) {
						if (!isalpha(container_patch[patch_iter].data[0])) {
							cout << container_patch[patch_iter].data;
						}
						break;
					}
					if (key_patch_iter == (int)container_key_patch.size() - 1) {
						if (!isalpha(container_patch[patch_iter].data[0])) {
							cout << container_patch[patch_iter].data;
						}
					}
				}
			} 
		}
		//break;
	}
	
	return 0;
}

