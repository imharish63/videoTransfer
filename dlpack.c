#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

// exact commit is in readme, master; https://github.com/dmlc/dlpack/blob/master/include/dlpack/dlpack.h
#include "dlpack.h"


void deleter(struct DLManagedTensor* self)
{
	fprintf(stderr, "Deleter calling\n");
	if(self->dl_tensor.data)
	{
		free(self->dl_tensor.data);
		self->dl_tensor.data = NULL;
	}

	if(self->dl_tensor.shape)
	{
		free(self->dl_tensor.shape);
		self->dl_tensor.shape = NULL;
	}
	
	if(self->dl_tensor.strides)
	{
		free(self->dl_tensor.strides);
		self->dl_tensor.strides = NULL;
	}
	fprintf(stderr, "Deleter called\n");
}

void create_and_allocate_dlpack_tensor(struct DLManagedTensor *dlpack, int64_t width, int64_t height, u8_t* image_frame_buf)
{
	printf("in function width %"PRId64",height %"PRId64"\r\n",width, height);
//	struct DLManagedTensor dlpack = {0};
	dlpack->deleter = deleter;
	dlpack->dl_tensor.ctx.device_type = kDLCPU;
	dlpack->dl_tensor.ndim = 4;
	xil_printf("no.of dimensions %x \r\n", dlpack->dl_tensor.ndim);
	DLDataType dtype = {.code = 0, .bits = 8, .lanes = 1};
	dlpack->dl_tensor.dtype = dtype;
	dlpack->dl_tensor.shape = malloc(dlpack->dl_tensor.ndim * sizeof(int64_t));
	dlpack->dl_tensor.shape[0] = (height/BATCH_SIZE_IMAGE);
	dlpack->dl_tensor.shape[1] = (width/BATCH_SIZE_IMAGE);
	dlpack->dl_tensor.shape[2] = BATCH_SIZE_IMAGE ;
	dlpack->dl_tensor.shape[3] = BATCH_SIZE_IMAGE ;
	printf("Dimensions (%"PRId64",%"PRId64",%"PRId64", %"PRId64") \r\n",dlpack->dl_tensor.shape[0], dlpack->dl_tensor.shape[1], dlpack->dl_tensor.shape[2], dlpack->dl_tensor.shape[3]);
	dlpack->dl_tensor.strides = malloc(dlpack->dl_tensor.ndim * sizeof(int64_t));
	dlpack->dl_tensor.strides[0] = dlpack->dl_tensor.shape[1] * dlpack->dl_tensor.shape[2] * dlpack->dl_tensor.shape[3];
	dlpack->dl_tensor.strides[1] = dlpack->dl_tensor.shape[2];
	dlpack->dl_tensor.strides[2] = dlpack->dl_tensor.strides[0] / dlpack->dl_tensor.strides[1];
	dlpack->dl_tensor.strides[3] = 1;
	printf("Strides (%"PRId64",%"PRId64",%"PRId64", %"PRId64") \r\n",dlpack->dl_tensor.strides[0], dlpack->dl_tensor.strides[1], dlpack->dl_tensor.strides[2],dlpack->dl_tensor.strides[3]);

//	uint64_t itemsize = dlpack.dl_tensor.dtype.lanes * dlpack.dl_tensor.dtype.bits / 8;
//	u8_t* data_ptr = calloc(dlpack.dl_tensor.shape[0] * dlpack.dl_tensor.shape[1], itemsize);
//	u8_t* data_ptr = image_frame_buf;
	// 3x2 int32 tensor with some sample data
//	data_ptr[0] = 0;
//	data_ptr[1] = 1;
//	data_ptr[2] = 2;
//	data_ptr[3] = 3;
//	data_ptr[4] = 4;
//	data_ptr[5] = 5;

	dlpack->dl_tensor.data = image_frame_buf;
	u8_t *data;
	data = (u8_t *)dlpack->dl_tensor.data;
	xil_printf(" dl_tensor data %x , address %x ,  and image_frame_buf %x , address %x \r\n "
			, data[0], dlpack->dl_tensor.data,  image_frame_buf[0], &image_frame_buf[0]);

	xil_printf("function exit \r\n");


}

//int main(int argc, char **argv)
//{
//	struct DLManagedTensor dlpack = create_and_allocate_dlpack_tensor();
//	uint64_t itemsize = dlpack.dl_tensor.dtype.lanes * dlpack.dl_tensor.dtype.bits / 8;
//	printf("after function  \r\n");
//	printf("data_ptr",dlpack->data_ptr);
//	//fwrite(dlpack.dl_tensor.data, itemsize, dlpack.dl_tensor.shape[0] * dlpack.dl_tensor.shape[1], fopen(argv[1], "wb"));
//	return 0;
//}
