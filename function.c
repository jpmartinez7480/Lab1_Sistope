#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include  <errno.h>

#include "function.h"


extern int errno;

/*
Funcion que abre abre el archivo con el nombre indicado en el parametro de entrada.
entrada: string que indica el nombre del archivo a leer.
salida: puntero al archivo a leer.
*/

int get_padding_size(int width){
	int padding = 0;
	int pitch = width*3;
	if(pitch % 4 != 0) pitch+= 4 - (pitch % 4);
	padding = pitch - (3*width);
	return padding;
}

FILE *open_bmp_file(char *bmp_file_name){
	FILE *file;
	if(!(file = fopen(bmp_file_name,"rb"))){
		fprintf(stderr,"Error opening the file: %s\n",strerror(errno));
		return NULL;
	}
	else{
		return file;
	}
}

/*
Funcion que lee el archivo bmp dado con el nombre dado como parametro.
entrada: string que indica el nombre del archivo
salida: puntero a un struct que representa la imagen leida.
*/

bmp_image *read_bmp_file(char *n_bmp_file){
	FILE *bmp_file;
	bmp_image *image;
	bmp_header header;
	int i,j;
	bmp_file = open_bmp_file(n_bmp_file);
	if(bmp_file){
		image = (bmp_image*)malloc(sizeof(bmp_image));
		fread(&header, sizeof(bmp_header), 1, bmp_file);
		image->header = header;
		image->pixel_array = malloc(image->header.height*sizeof(Pixel*));
		for(i= 0; i < image->header.height; i++)
			image->pixel_array[i] = malloc(image->header.width*sizeof(Pixel));
		fseek(bmp_file,header.offset,SEEK_SET);
		switch(image->header.bits_per_pixel){
			case 24:
				if(image->header.width % 4 != 0){
					int padding = get_padding_size(header.width);
					for(i = 0; i < image->header.height; i++){
						for(j = 0; j < image->header.width; j++){
							fread(&image->pixel_array[i][j].b, sizeof(char), 1, bmp_file);
							fread(&image->pixel_array[i][j].g, sizeof(char), 1, bmp_file);
							fread(&image->pixel_array[i][j].r, sizeof(char), 1, bmp_file);
						}
						fread(&image->pixel_array[i][j].p,sizeof(char),padding,bmp_file);
					}
				}
	
				else{
					for(i = 0; i < image->header.height; i++){
						for(j = 0; j < image->header.width; j++){
							fread(&image->pixel_array[i][j].b, sizeof(char), 1, bmp_file);
							fread(&image->pixel_array[i][j].g, sizeof(char), 1, bmp_file);
							fread(&image->pixel_array[i][j].r, sizeof(char), 1, bmp_file);
						}
					}
				}
				break;

			case 32:
				for(i = 0; i < image->header.height; i++){
					for(j = 0; j < image->header.width; j++){
						fread(&image->pixel_array[i][j].b, sizeof(char), 1, bmp_file);
						fread(&image->pixel_array[i][j].g, sizeof(char), 1, bmp_file);
						fread(&image->pixel_array[i][j].r, sizeof(char), 1, bmp_file);
						fread(&image->pixel_array[i][j].p, sizeof(char), 1, bmp_file);
					}
				}
				break;
		}
		fclose(bmp_file);
		return image;
	}
	else return NULL;	
}

/*
Funcion que crea una imagen bmp
entrada: puntero a struct que contiene los datos de la imagen a escribir y nombre del archivo
*/

void write_bmp_file(bmp_image *image, char *bmp_file){
	int i,j;
	int padding;
	char aux1[30] = "res_";
	strcat(aux1,bmp_file);
	FILE *aux = fopen(aux1,"wb");
	fwrite(&image->header,sizeof(bmp_header),1,aux);
	fseek(aux,image->header.offset,SEEK_SET);
	switch(image->header.bits_per_pixel){
		case 24:
			if(image->header.width % 4 != 0){
				int padding = get_padding_size(image->header.width); 
				for(i = 0; i < image->header.height; i++){
					for(j = 0; j < image->header.width; j++){
						fwrite(&image->pixel_array[i][j].b,sizeof(char),1,aux);
						fwrite(&image->pixel_array[i][j].g,sizeof(char),1,aux);
						fwrite(&image->pixel_array[i][j].r,sizeof(char),1,aux);
					}
					fwrite(&image->pixel_array[i][j].p,sizeof(char),padding,aux);
				}
			}
			else{
				for(i = 0; i < image->header.height; i++){
					for(j  = 0; j < image->header.width; j++){
						fwrite(&image->pixel_array[i][j].b,sizeof(char),1,aux);
						fwrite(&image->pixel_array[i][j].g,sizeof(char),1,aux);
						fwrite(&image->pixel_array[i][j].r,sizeof(char),1,aux);
					}
				}
			}
			break;
		case 32:	
			for(i = 0; i < image->header.height; i++){
				for(j = 0; j < image->header.width; j++){
					fwrite(&image->pixel_array[i][j].b,sizeof(char),1,aux);
					fwrite(&image->pixel_array[i][j].g,sizeof(char),1,aux);
					fwrite(&image->pixel_array[i][j].r,sizeof(char),1,aux);
					fwrite(&image->pixel_array[i][j].p,sizeof(char),1,aux);
				}
			}
			break;
		
		fclose(aux);
	}
}

/*
Funcion que convierte la imagen indicada como parametro a escala de grises.
entrada: puntero a un struct que indica la imagen a convertir.
*/

void to_gray_scale(bmp_image *image){
	int i,j;
	for(i = 0; i < image->header.height; i++){
		for(j = 0; j < image->header.width; j++){
			int gray = image->pixel_array[i][j].r*0.3 + image->pixel_array[i][j].g*0.59 + image->pixel_array[i][j].b*0.11; 
			image->pixel_array[i][j].r = gray;
			image->pixel_array[i][j].g = gray;
			image->pixel_array[i][j].b = gray;
		}
	}
}

/*
Funcion que transforma la imagen en escala de grises dada como parametro y segun un umbral, binariza la imagen
entrada: puntero a un struct que representa la imagen en escala de grises y un entero que indica el umbral 
*/

void to_binary_image(bmp_image *image, int umbral, char *bmp_file){
	int i,j; 
	for(i = 0; i < image->header.height; i++){
		for(j = 0; j < image->header.width; j++){
			if(image->pixel_array[i][j].b > umbral){
				image->pixel_array[i][j].b = 255;
				image->pixel_array[i][j].g = 255;
				image->pixel_array[i][j].r = 255;
			}
			else{
				image->pixel_array[i][j].b = 0;
				image->pixel_array[i][j].g = 0;
				image->pixel_array[i][j].r = 0;
			} 
		}
	}
	write_bmp_file(image,bmp_file);
}

/*
Funcion que recibe un puntero a un struct que representa la imagen y el valor del porcentaje a comparar para
analizar si la imagen es nearly black o no.
entrada: puntero a struct que reprenta la imagen y entero que indica el valor del porcentaje.
salida: entero que indica si la imagen es nearly black o no (1 si lo es, 0 no lo es).
*/

int sort_out_image_binary(bmp_image *image, int c){
	int i,j;
	int px_black = 0;
	for(i = 0; i < image->header.height; i++){
		for(j = 0; j < image->header.width; j++){
			if(image->pixel_array[i][j].b == 0) px_black++;
		}
	}
	if(((px_black*100)/(image->header.height*image->header.width)) >= c) return 1;
	else return 0;
}

/*
Funcion que recibe el nombre del archivo analizado y un valor booleano que indica si la imagen
es nearly black o no, con el fin de imprimir por pantalla el nombre del archivo y si es nearly black o no.
entrada: string que contiene el nombre del archivo analizado y un entero que indica si la imagen es nearly black o no.
*/

void write_result(char *bmp_file, int value){
	if(value == 1) printf("|   %-5s   |   yes            |\n",bmp_file);
	else printf("|   %-5s   |   no             |\n",bmp_file);
}

/*
Funcion que ejecuta un pipeline para la cantidad de imagenes indicadas en el parametro, realizando un
analisis segun los demas parametros dados.
entrada: amount_images representa la cantidad de imagenes a analizar, umbral indica el umbral a comparar
para binarizar la imagen, cat representa el porcentaje a comparar para saber si la imagen es nearly black
y flag indica si los resultados seran mostrados o no.
*/


void execute_task(int amount_images, int umbral, int cat, int flag){
	
	int cnt = 1;
	int enabled = 0;
	int status = 0;
	while(cnt <= amount_images){
		char nfile[20] = "imagen_";
		char format[] = ".bmp";
		char n[2];
		bmp_image *image;
		sprintf(n,"%d",cnt);
		strcat(nfile,n);
		strcat(nfile,format);

		image = read_bmp_file(nfile);
		to_gray_scale(image);
		to_binary_image(image,umbral,nfile);
		status = sort_out_image_binary(image,cat);
		if(flag == 1) write_result(nfile, status);

		free(image);
		cnt+=1;
	}
}

