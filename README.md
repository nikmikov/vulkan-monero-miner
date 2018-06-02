# dorenom


## System Requirements

CPU with AES-NI support for CPU mining


## Dependencies

cJSON: https://github.com/DaveGamble/cJSON

libuv: http://libuv.org/ [github](https://github.com/libuv/libuv)


## Debugging

To enable vulkan validation and debug layers run debug build with

```
VK_LAYER_PATH=/usr/share/vulkan/explicit_layer.d VK_LOADER_DEBUG=all dorenom --config src/config.template --bench

```
