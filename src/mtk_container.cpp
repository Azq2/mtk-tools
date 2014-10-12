#include "mtk_container.hpp"

int help() {
	printf(
		"usage: %s\n"
		"\t  -i|--input container.img\n"
		"\t[ -d|--unpack ] unpack container\n"
		"\t[ -t|--type container_type ]\n"
		"\t[ -o|--output output_directory_or_file ]\n", 
		prog_name
	);
	return 0;
}

int main(int argc, char **argv) {
	int c;
	int option_index = 0;
	
	prog_name = argv[0];
	
	bool unpack = false;
	const char *type = NULL;
	const char *input_file = NULL;
	const char *output_file = NULL;
	
	static struct option long_options[] = {
		{"unpack", no_argument, 0, 'd' }, 
		{"type", required_argument, 0, 't' }, 
		{"input", required_argument, 0, 'i' }, 
		{"output", required_argument, 0, 'o' }, 
	};
	
	while ((c = getopt_long(argc, argv, "dt:i:o:", long_options, &option_index)) != -1) {
		if (c == 'd') {
			unpack = true;
		} else if (c == 'i') {
			input_file = optarg;
		} else if (c == 'o') {
			output_file = optarg;
		} else if (c == 't') {
			type = optarg;
		}
	}
	
	unsigned char *data;
	unsigned int length = 0;
	if (!input_file || !(data = read_file(input_file, &length)) || (!unpack && !type))
		return help();
	
	char *out_filename = (char *) malloc(strlen(input_file) + (output_file ? strlen(output_file) : 0) + 32);
	if (unpack) {
		mtk_container c;
		
		if (length < sizeof(struct mtk_container_hdr) || memcmp(data, clen(MTK_CONTAINER)) != 0) {
			free(data);
			fprintf(stderr, "File %s is not MTK container!\n", input_file);
			return 1;
		}
		
		c.header = (struct mtk_container_hdr *) data;
		c.data   = (unsigned char *) (data + sizeof(struct mtk_container_hdr));
		c.length = c.header->data_length;
		
		if (length < sizeof(struct mtk_container_hdr) + c.length) {
			free(data);
			fprintf(stderr, "File %s corrupted!\n", input_file);
			return 1;
		}
		
		printf("unpacking...\n");
		printf("type: %s\n", c.header->data_type);
		printf("size: 0x%08X\n", c.header->data_length);
		
		if (output_file && is_dir(output_file)) {
			sprintf(out_filename, "%s/%s", output_file, basename((char *) input_file));
		} else if (output_file) {
			sprintf(out_filename, "%s", output_file);
		} else
			sprintf(out_filename, "./%s.raw", basename((char *) input_file));
		write_file(out_filename, 1, c.data, c.length);
	} else {
		mtk_container_hdr hdr;
		memset(hdr.data_type, 0, sizeof(hdr.data_type));
		memset(hdr.reserved, 0xFF, sizeof(hdr.reserved));
		
		printf("packing...\n");
		printf("type: %s\n", type);
		printf("size: 0x%08X\n", length);
		
		hdr.data_length = length;
		
		memcpy(hdr.magic, MTK_CONTAINER, sizeof(MTK_CONTAINER));
		strcpy(hdr.data_type, type);
		
		if (output_file && is_dir(output_file)) {
			sprintf(out_filename, "%s/%s", output_file, basename((char *) input_file));
		} else if (output_file) {
			sprintf(out_filename, "%s", output_file);
		} else
			sprintf(out_filename, "./%s.bin", basename((char *) input_file));
		
		write_file(out_filename, 2, &hdr, sizeof(hdr), data, length);
	}
	free(out_filename);
	free(data);
	
	return 0;
}
