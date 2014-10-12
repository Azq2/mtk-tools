#include "main.hpp"

int main(int argc, char **argv) {
	unsigned int page_size = 0;
	const char *filename = NULL;
	const char *out_dir = NULL;
	
	prog_name = argv[0];
	
	argc--;
	argv++;
	
	if (argc % 2 == 0) {
		while (argc > 0) {
			char *arg = argv[0];
			char *val = argv[1];
			argc -= 2; argv += 2;
			
			if (!strcmp(arg, "--input") || !strcmp(arg, "-i")) {
				filename = val;
			} else if (!strcmp(arg, "--output") || !strcmp(arg, "-o")) {
				out_dir = val;
			} else if (!strcmp(arg, "--pagesize") || !strcmp(arg, "-p")) {
				page_size = strtoul(val, 0, 16);
			} else
				return help();
		}
	}
	
	unsigned int length = 0;
	unsigned char *data;
	
	if (!filename || !(data = read_file(filename, &length)))
		return help();
	
	if (length < sizeof(android_boot_img_hdr) && length < sizeof(mtk_container_hdr)) {
		fprintf(stderr, "File corrupted! (can't read header)\n");
		return 1;
	}
	
	struct android_boot_img img;
	if (memcmp(data, clen(BOOT_MAGIC)) == 0) { // Это android boot.img
	//	printf("THIS IS SPARTAAAA^W ANDROID! (%d)\n", length);
		
		img.header = (struct android_boot_img_hdr *) data;
		if (page_size == 0)
			page_size = img.header->page_size;
		img.data   = (unsigned char *) (data + page_size);
		img.length = length - sizeof(struct android_boot_img_hdr);
		
		dump_android_img_hdr(img.header, page_size);
		
		unsigned int kernel_size  = (img.header->kernel_size + page_size - 1) / page_size  * page_size;
		unsigned int ramdisk_size = (img.header->ramdisk_size + page_size - 1) / page_size * page_size;
		unsigned int second_size  = (img.header->second_size + page_size - 1) / page_size  * page_size;
		
		bool need_free = false;
		unsigned char *save_data = NULL;
		unsigned int save_data_length = 0;
		
		if (img.length < kernel_size + ramdisk_size + second_size) {
			fprintf(stderr, "File corrupted! (can't read data)\n");
			return 1;
		}
		
		if (!out_dir)
			out_dir = ".";
		
		char addr_tmp[32];
		char *out_filename = (char *) malloc(strlen(filename) + strlen(out_dir) + 32);
		
		sprintf(out_filename, "%s/%s-cmdline", out_dir, basename((char *) filename));
		write_file(out_filename, img.header->cmdline, strlen(img.header->cmdline));
		
		sprintf(addr_tmp, "%08x", page_size);
		sprintf(out_filename, "%s/%s-pagesize", out_dir, basename((char *) filename));
		write_file(out_filename, addr_tmp, strlen(addr_tmp));
		
		sprintf(addr_tmp, "%08x", img.header->kernel_addr - 0x00008000);
		sprintf(out_filename, "%s/%s-base", out_dir, basename((char *) filename));
		write_file(out_filename, addr_tmp, strlen(addr_tmp));
		
		if (kernel_size) {
			save_data = unpack_data(img.data, kernel_size, &save_data_length, &need_free);
			
			sprintf(out_filename, "%s/%s-zImage", out_dir, basename((char *) filename));
			write_file(out_filename, save_data, save_data_length);
			
			if (need_free)
				free(save_data);
		}
		
		if (ramdisk_size) {
			save_data = unpack_data(img.data + kernel_size, ramdisk_size, &save_data_length, &need_free);
			
			sprintf(out_filename, "%s/%s-ramdisk.gz", out_dir, basename((char *) filename));
			write_file(out_filename, save_data, save_data_length);
			
			if (need_free)
				free(save_data);
		}
		
		if (second_size) {
			save_data = unpack_data(img.data + kernel_size + ramdisk_size, second_size, &save_data_length, &need_free);
			
			sprintf(out_filename, "%s/%s-second.img", out_dir, basename((char *) filename));
			write_file(out_filename, save_data, save_data_length);
			
			if (need_free)
				free(save_data);
		}
		
		free(out_filename);
	} else if (memcmp(data, clen(MTK_CONTAINER)) == 0) { // MTK говно странное
		mtk_container c;
		
		c.header = (struct mtk_container_hdr *) data;
		c.data   = (unsigned char *) (data + sizeof(struct mtk_container_hdr));
		c.length = c.header->data_length;
		
		printf("type: %s\n", c.header->data_type);
		printf("size: 0x%08X\n", c.header->data_length);
		
		char *out_filename = (char *) malloc(strlen(filename) + strlen(out_dir) + 32);
		if (out_dir && is_dir(out_dir)) {
			sprintf(out_filename, "%s/%s", out_dir, basename((char *) filename));
		} else if (out_dir) {
			sprintf(out_filename, "%s", out_dir);
		} else
			sprintf(out_filename, "./%s.raw", basename((char *) filename));
		write_file(out_filename, c.data, c.length);
		free(out_filename);
	} else {
		fprintf(stderr, "Unknown file type!\n");
		return 1;
	}
	
	return 0;
}

int help() {
	printf(
		"usage: %s\n"
		"\t  -i|--input boot.img\n"
		"\t[ -o|--output output_directory ]\n"
		"\t[ -p|--pagesize <size-in-hexadecimal> ]\n"
		"\n   OR   \n"
		"\t  -i|--input mtk_container.img\n"
		"\t[ -o|--output data.raw ]\n", 
		prog_name
	);
	return 0;
}

unsigned char *unpack_data(const unsigned char *data, unsigned int size, unsigned int *new_size, bool *need_free) {
	if (need_free) *need_free = false; // Пока не нужно
	
	if (memcmp(data, clen(MTK_CONTAINER)) == 0) { // Хуитка ёбанная
		mtk_container c;
		
		c.header = (struct mtk_container_hdr *) data;
		c.data   = (unsigned char *) (data + sizeof(struct mtk_container_hdr));
		c.length = c.header->data_length;
		
	//	dump_mtk_container_hdr(c.header);
		
		*new_size = size;
		return c.data;
	}
	*new_size = size;
	return (unsigned char *) data;
}

void dump_mtk_container_hdr(struct mtk_container_hdr *header) {
	printf(
		"mtk_container_hdr {\n"
		"  data_type = %s\n"
		"  data_length = 0x%08X\n"
		"}\n", 
		header->data_type, 
		header->data_length
	);
}

void dump_android_img_hdr(struct android_boot_img_hdr *header, unsigned int ps) {
	if (!ps)
		ps = header->page_size;
	printf(
		"kernel_size: 0x%08X (%d pages)\n"
		"kernel_addr: 0x%08X\n"
		"kernel_base: 0x%08X\n"
		"\n"
		"ramdisk_size: 0x%08X (%d pages)\n"
		"ramdisk_addr: 0x%08X\n"
		"\n"
		"second_size: 0x%08X (%d pages)\n"
		"second_addr: 0x%08X\n"
		"\n"
		"tags_addr: 0x%08X\n"
		"page_size: 0x%08X\n"
		"\n"
		"id: 0x%02X%02X%02X%02X%02X%02X%02X%02X\n"
		"name: \"%s\"\n"
		"cmdline: \"%s\"\n"
		"extra_cmdline: \"%s\"\n", 
		
		header->kernel_size, (header->kernel_size + ps - 1) / ps, 
		header->kernel_addr, 
		header->kernel_addr - 0x00008000, 
		header->ramdisk_size, (header->ramdisk_size + ps - 1) / ps, 
		header->ramdisk_addr, 
		header->second_size, (header->second_size + ps - 1) / ps, 
		header->second_addr, 
		
		header->tags_addr, 
		header->page_size, 
		
		header->id[0], header->id[1], header->id[2], header->id[3], 
		header->id[4], header->id[5], header->id[6], header->id[7], 
		header->name, 
		header->cmdline, 
		header->extra_cmdline
	);
}

bool write_file(const char *path, const void *data, unsigned int length) {
	FILE *fp = NULL;
	
	do {
		if (!(fp = fopen(path, "w+"))) {
			perror(path);
			break;
		}
		if (fwrite(data, 1, length, fp) != length) {
			perror(path);
			break;
		}
		fclose(fp);
		return true;
	} while (0);
	
	if (fp)
		fclose(fp);
	return false;
}

unsigned char *read_file(const char *path, unsigned int *length) {
	struct stat st;
	
	FILE *fp = NULL;
	unsigned char *data = NULL;
	
	*length = 0;
	
	do {
		if (!(fp = fopen(path, "r"))) {
			perror(path);
			break;
		}
		if (fstat(fileno(fp), &st) != 0 || !S_ISREG(st.st_mode) || st.st_size == 0) {
			perror(path);
			break;
		}
		data = (unsigned char *) malloc(st.st_size);
		if (fread(data, 1, st.st_size, fp) != st.st_size) {
			perror(path);
			break;
		}
		fclose(fp);
		
		*length = st.st_size;
		
		return data;
	} while (0);
	
	if (data)
		free(data);
	if (fp)
		fclose(fp);
	
	return NULL;
}

bool is_dir(const char *path) {
	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}
