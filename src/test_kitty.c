#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define WIDTH  100  // Width of the image
#define HEIGHT 300 // Height of the image
#define CHUNK_SIZE 4096 // Max size per chunk for Kitty protocol

// Function to encode pixel data in base64
char *base64_encode(const uint8_t *data, size_t input_length, size_t *output_length);

// Function to send image in chunks using Kitty protocol
void send_image_to_kitty(const uint8_t *pixels, int width, int height);

int main() {
    uint8_t *pixels = malloc(3 * WIDTH * HEIGHT);
    if (pixels == NULL) {
        perror("Failed to allocate memory for pixels");
        return EXIT_FAILURE;
    }

    // Create a simple gradient for demonstration
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            size_t index = 3 * (y * WIDTH + x);
            pixels[index + 0] = x * 255 / WIDTH;  // Red
            pixels[index + 1] = y * 255 / HEIGHT; // Green
            pixels[index + 2] = 128;              // Blue
						printf("%d\n", pixels[index]);
						printf("%d\n", pixels[index+1]);
						printf("%d\n", pixels[index+2]);
        }
    }

		const uint8_t aux[]= "\xff\x00\x00\x00\xff\x00\x00\x00\xff";
    // Display image in Kitty terminal
    send_image_to_kitty(pixels, WIDTH, HEIGHT);
    //send_image_to_kitty(aux, WIDTH, HEIGHT);

    // Free memory
    free(pixels);
    return 0;
}
#include <stdlib.h>
#include <stdint.h>

char *base64_encode(const uint8_t *data, size_t input_length, size_t *output_length) {
    static const char encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t output_len = 4 * ((input_length + 2) / 3); // Ensure output is multiple of 4
    char *encoded_data = malloc(output_len + 1); // +1 for null-termination
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        encoded_data[j++] = encoding_table[(triple >> 18) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 12) & 0x3F];
        encoded_data[j++] = (i > input_length + 1) ? '=' : encoding_table[(triple >> 6) & 0x3F];
        encoded_data[j++] = (i > input_length) ? '=' : encoding_table[triple & 0x3F];
    }

    encoded_data[output_len] = '\0';  // Null-terminate the string
    *output_length = output_len;
    return encoded_data;
}

// Send the image data in chunks if necessary
void send_image_to_kitty(const uint8_t *pixels, int width, int height) {
    size_t pixel_data_length = 3 * width * height;
    size_t base64_length;
    char *base64_pixels = base64_encode(pixels, pixel_data_length, &base64_length);
		printf("Base64: %s\n", base64_pixels);
		printf("Base 64 length: %d\n", base64_length);
		printf("Pixels: %s\n", pixels);
    if (base64_pixels == NULL) {
        perror("Failed to encode pixels in base64");
        return;
    }

    size_t offset = 0;
    while (base64_length > 0) {
        // Calculate chunk size (4096 or remaining length)
        size_t payloadToRead = (base64_length > CHUNK_SIZE) ? CHUNK_SIZE : base64_length;
        int m = (base64_length > CHUNK_SIZE) ? 1 : 0;

        // Print escape sequence for Kitty protocol, using `a=d`
				if(offset == 0){
					printf("\x1B_Gf=24,a=T,s=%d,i=2,v=%d,m=%d;%.*s\x1B\\", width, height, m, (int)payloadToRead, base64_pixels + offset);
				}else{  
					printf("\x1B_Gm=%d;%.*s\x1B\\", m, (int)payloadToRead, base64_pixels + offset);
				}  
			//printf("_Ga=d,m=%d,f=24,s=%d,v=%d;%.*s\n", m, width, height, (int)payloadToRead, base64_pixels + offset);
        fflush(stdout);  // Flush output to ensure it’s sent immediately

        base64_length -= payloadToRead;
        offset += payloadToRead;
    }

    free(base64_pixels);
}

