#include "common.hpp"

unsigned char *unpack_data(const unsigned char *data, unsigned int size, unsigned int *new_size, bool *need_free) {
	if (need_free) *need_free = false; // Пока не нужно
	
	if (memcmp(data, clen(MTK_CONTAINER)) == 0) { // Хуитка ёбанная
		mtk_container c;
		
		c.header = (struct mtk_container_hdr *) data;
		c.data   = (unsigned char *) (data + sizeof(struct mtk_container_hdr));
		c.length = c.header->data_length;
		
	//	dump_mtk_container_hdr(c.header);
		
		*new_size = c.length;
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

bool write_file(const char *path, unsigned int n, ...) {
	FILE *fp = NULL;
	
	do {
		if (!(fp = fopen(path, "w+"))) {
			perror(path);
			break;
		}
		
		void *data;
		unsigned int length;
		
		n *= 2;
		
		va_list vl;
		va_start(vl, n);
		for (unsigned int i = 0; i < n; i += 2) {
			data = va_arg(vl, void *);
			length = va_arg(vl, unsigned int);
			
			if (fwrite(data, 1, length, fp) != length) {
				perror(path);
				break;
			}
		}
		va_end(vl);
		
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
