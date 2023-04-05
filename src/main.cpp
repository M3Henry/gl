#include <iostream>
#include <chrono>
#include <thread>
#include <unistd.h>
#include "hulkan.hpp"
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>
#include <cstddef>
#include <array>
#include <bitset>
#include <glm/glm.hpp>

struct vertex_t
{
	glm::vec2 pos;
	glm::vec3 col;

	const static vk::VertexInputBindingDescription bindings;
	const static std::vector<vk::VertexInputAttributeDescription> attributes;
};

const vk::VertexInputBindingDescription vertex_t::bindings
(
	0,
	sizeof(vertex_t),
	vk::VertexInputRate::eVertex
);

const std::vector<vk::VertexInputAttributeDescription> vertex_t::attributes =
{
	{0, 0, vk::Format::eR32G32Sfloat, offsetof(vertex_t, pos)},
	{1, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex_t, col)}
};

const std::vector<vertex_t> vertices =
{
	{{ 0.0f,-0.5f}, {1.0f, 0.0f, 0.0f}},
	{{ 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
	
	{{ 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{ 0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}}
};

// PROGRAM

extern uint32_t _binary_shader_vert_spv_end;
extern uint32_t _binary_shader_vert_spv_start;
auto vertexObject = make_file(_binary_shader_vert_spv_start, _binary_shader_vert_spv_end);

extern uint32_t _binary_shader_frag_spv_end;
extern uint32_t _binary_shader_frag_spv_start;
auto fragmentObject = make_file(_binary_shader_frag_spv_start, _binary_shader_frag_spv_end);

glfw::window mainWindow;

float color = 0.f;
int main()
{
	std::cout << std::boolalpha << "Hello, world!\n" << std::endl;

	glfwWindowHint(GLFW_DOUBLEBUFFER, true);
	glfwWindowHint(GLFW_VISIBLE, false);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	mainWindow = glfw::window(800, 600, "Hello GLFW");
	glfwSwapInterval(1);
	mainWindow.makeContextCurrent();
	mainWindow.setKeyCallback
	(
		[](glfw::window& window, int key, int scancode, int action, int mods)
		{
			if (action == GLFW_PRESS) {
				std::cout << char(key);
				std::cout.flush();
					switch (key) {
						case GLFW_KEY_ESCAPE:
							window.shouldClose(true);
							return;
				case GLFW_KEY_C:
					color = color < .5f ? 1.f : 0.f;
					return;
				default:
					return;
				}
			}
			return;
		}
	);
	// vulkan
	try
	{
		auto& instance = hulkan::instance();

		auto surface = mainWindow.createSurface(instance);

		auto physicalDevice = hulkan::initialisePhysicalDevice(surface);

		auto queueFamily = [&physicalDevice, &surface]()
		{
			int graphicsFamily = -1;
			int presentationFamily = -1;
			auto queueProperties = physicalDevice.getQueueFamilyProperties();
			int i = 0;
			for (auto &q : queueProperties)
			{
				if (q.queueCount > 0)
				{
					if (q.queueFlags & vk::QueueFlagBits::eGraphics) graphicsFamily = i;
					if (physicalDevice.getSurfaceSupportKHR(i, surface)) presentationFamily = i;
				}
				if (graphicsFamily >= 0 && presentationFamily >= 0) break;
				++i;
			}
			std::cout << "graphicsFamily = " << graphicsFamily << std::endl;
			std::cout << "presentationFamily = " << presentationFamily << std::endl;
			if (graphicsFamily < 0 || presentationFamily < 0)
				throw std::runtime_error("a queue family is missing");
			return std::make_pair<uint32_t, uint32_t>(graphicsFamily, presentationFamily);
		}();

		auto logicalDevice = [&physicalDevice, &queueFamily]()
		{
			std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueFamilies = {queueFamily.first, queueFamily.second};
			float queuePriority = 1.f;
			for (auto qFam : uniqueFamilies)
			{
				queueCreateInfos.emplace_back(
					vk::DeviceQueueCreateFlags(),
					qFam, 1, &queuePriority
				);
			}
			const std::vector<const char*> deviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
			auto deviceFeatures = physicalDevice.getFeatures();

			auto dci = vk::DeviceCreateInfo
			(
				vk::DeviceCreateFlags(),
				queueCreateInfos.size(), queueCreateInfos.data(),
				0, nullptr, // validation layers
				deviceExtensions.size(), deviceExtensions.data(), // enabled extensions,
				&deviceFeatures
			);
			return physicalDevice.createDevice(dci);
		}();

		auto commandPool = logicalDevice.createCommandPool
		({
			vk::CommandPoolCreateFlags(),
			queueFamily.first
			// command pool flags
		});
		std::cout << "Created command pool\n";

		auto graphicsQueue = logicalDevice.getQueue(queueFamily.first, 0); // 0 = queue index

		auto presentationQueue = logicalDevice.getQueue(queueFamily.second, 0); // 0 = queue index

		auto pipelineLayout = [&logicalDevice]()
		{
			auto plci = vk::PipelineLayoutCreateInfo
			(
				vk::PipelineLayoutCreateFlags(),
				0, nullptr,	// layouts
				0, nullptr	// push constant ranges
			);
			return logicalDevice.createPipelineLayout(plci);
		}();

		auto surfaceFormat = [&physicalDevice, &surface]() -> vk::SurfaceFormatKHR
		{
			auto availableFormats = physicalDevice.getSurfaceFormatsKHR(surface);
			auto desiredFormat = vk::Format::eB8G8R8A8Unorm;
			auto desiredColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
			if (availableFormats.size() == 1 && availableFormats.front().format == vk::Format::eUndefined)
			{
				return {desiredFormat, desiredColorSpace};
			}
			for (const auto & f : availableFormats)
			{
				if (f.format == desiredFormat && f.colorSpace == desiredColorSpace) return f;
			}
			return availableFormats.front();
		}();
		std::cout << "SurfaceFormat = " << to_string(surfaceFormat.format) << "  Colourspace = " << to_string(surfaceFormat.colorSpace) << '\n';

		auto imageAvailable = logicalDevice.createSemaphore({});
		auto renderFinished = logicalDevice.createSemaphore({});

		vk::SwapchainKHR swapChain;
		vk::Extent2D swapExtent;
		swapChain = [&swapChain, &swapExtent, &physicalDevice, &logicalDevice, &surface, &surfaceFormat, &queueFamily]()
		{
			auto presentationMode = [&physicalDevice, &surface]()
			{
				std::cout << "Available presentation modes:\n";
				auto availableModes = physicalDevice.getSurfacePresentModesKHR(surface);
				for (auto & m : availableModes)
				{
					std::cout << "	" << to_string(m) << '\n';
				}
				return vk::PresentModeKHR::eFifo;
			}();

			auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

			swapExtent = [&swapExtent, &capabilities]() -> vk::Extent2D
			{
				std::cout << "currentExtent = " << capabilities.currentExtent.width << 'x' << capabilities.currentExtent.height << std::endl;
				if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
					return capabilities.currentExtent;
				} else {
					auto [w, h] = mainWindow.getFramebufferSize();
					uint32_t width = w, height = h;

					std::cout << "minImageExtent = " << capabilities.minImageExtent.width << 'x' << capabilities.minImageExtent.height << std::endl;
					std::cout << "maxImageExtent = " << capabilities.maxImageExtent.width << 'x' << capabilities.maxImageExtent.height << std::endl;
					return {
						std::max(
							capabilities.minImageExtent.width,
							std::min(
								capabilities.maxImageExtent.width,
								width
							)
						),
						std::max(
							capabilities.minImageExtent.height,
							std::min(
								capabilities.maxImageExtent.height,
								height
							)
						)
					};
				}
			}();

			auto swapDepth = capabilities.minImageCount;
			if (0 == swapDepth)
				swapDepth = 2;
			std::cout << "Selected swap extent = " << swapExtent.width << 'x' << swapExtent.height << " : " << swapDepth << '\n';

			auto exclusiveMode = queueFamily.first == queueFamily.second;
			if (not exclusiveMode)
				throw std::runtime_error("Concurrent mode unsupported atm.");
			std::vector<uint32_t> queueFamilyArray = {queueFamily.first, queueFamily.second};

			auto newSwapChain = logicalDevice.createSwapchainKHR
			({
				vk::SwapchainCreateFlagsKHR(),
				surface,
				swapDepth,
				surfaceFormat.format, surfaceFormat.colorSpace,
				swapExtent,
				1, // image layers higher for stereoscopic
				vk::ImageUsageFlagBits::eColorAttachment,
				exclusiveMode ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent,
				(uint32_t)queueFamilyArray.size(),
				queueFamilyArray.data(),
				capabilities.currentTransform,
				vk::CompositeAlphaFlagBitsKHR::eOpaque,
				presentationMode,
				true,
				swapChain
			});
			logicalDevice.destroySwapchainKHR(swapChain);
			return newSwapChain;
		}();

		auto swapImages = logicalDevice.getSwapchainImagesKHR(swapChain);
		std::cout << "Created swap chain with length " << swapImages.size() << '\n';

		auto imageViews = [&logicalDevice, &swapImages, &surfaceFormat]()
		{
			std::vector<vk::ImageView> imageViews;
			for (auto & i : swapImages)
			{
				auto ivci = vk::ImageViewCreateInfo
				(
					vk::ImageViewCreateFlags(),
					i,
					vk::ImageViewType::e2D,
					surfaceFormat.format,
					vk::ComponentMapping(),
					vk::ImageSubresourceRange
					(
						vk::ImageAspectFlagBits::eColor,
						0,1, 	// mipmap levels
						0,1		// array layers (higher for stereographic)
					)
				);
				imageViews.push_back(logicalDevice.createImageView(ivci));
			}
			std::cout << "Created " << imageViews.size() << " image views." << std::endl;
			return imageViews;
		}();

		auto renderPass = [&logicalDevice, &surfaceFormat]()
		{
			auto colourAttachment = vk::AttachmentDescription
			(
				vk::AttachmentDescriptionFlags(),
				surfaceFormat.format,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,	// render data
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,	// stencil data
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
			);
			auto colourAttachmentRef = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);

			auto subpass = vk::SubpassDescription
			(
				vk::SubpassDescriptionFlags(),
				vk::PipelineBindPoint::eGraphics,
				0, nullptr,	// input
				1, &colourAttachmentRef // color
				// etc
			);
			auto dependency = vk::SubpassDependency
			(
				VK_SUBPASS_EXTERNAL, 0,
				vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::AccessFlags(), vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
			);
			auto rpci = vk::RenderPassCreateInfo
			(
				vk::RenderPassCreateFlags(),
				1, &colourAttachment,
				1, &subpass,
				1, &dependency // dependencies
			);
			return logicalDevice.createRenderPass(rpci);
		}();
		std::cout << "Created render pass" << std::endl;

		auto [result, graphicsPipeline] = [&pipelineLayout, &renderPass, &logicalDevice, &swapExtent]()
		{
			auto objReader = [&logicalDevice](std::pair<size_t, uint32_t*> code)
			{
				std::cout << "Reading " << code.first << " bytes from " << code.second << std::endl;
				auto smci = vk::ShaderModuleCreateInfo
				(
					vk::ShaderModuleCreateFlags(),
					code.first,	code.second
				);
				return logicalDevice.createShaderModule(smci);
			};
			auto vertShaderCode = objReader(::vertexObject);
			auto fragShaderCode = objReader(::fragmentObject);
			std::vector<vk::PipelineShaderStageCreateInfo> stages;
			stages.emplace_back
			(
				vk::PipelineShaderStageCreateFlags(),
				vk::ShaderStageFlagBits::eVertex,
				vertShaderCode,
				"main",
				nullptr	// specialization info (for constants)
			);
			stages.emplace_back
			(
				vk::PipelineShaderStageCreateFlags(),
				vk::ShaderStageFlagBits::eFragment,
				fragShaderCode,
				"main",
				nullptr	// specialization info (for constants)
			);
			auto pvisci = vk::PipelineVertexInputStateCreateInfo
			(
				vk::PipelineVertexInputStateCreateFlags(),
				1, &::vertex_t::bindings,
				::vertex_t::attributes.size(), ::vertex_t::attributes.data()
			);
			auto piasci = vk::PipelineInputAssemblyStateCreateInfo
			(
				vk::PipelineInputAssemblyStateCreateFlags(),
				vk::PrimitiveTopology::eTriangleList,
				false
			);
			auto viewport = vk::Viewport
			(
				0.f, 0.f,	// xy centre
				swapExtent.width, swapExtent.height,
				0.f, 1.f	// z extent
			);
			auto scissor = vk::Rect2D({0, 0}, swapExtent);

			auto pvsci= vk::PipelineViewportStateCreateInfo
			(
				vk::PipelineViewportStateCreateFlags(),
				1, &viewport,
				1, &scissor
			);
			auto rasterizer = vk::PipelineRasterizationStateCreateInfo
			(
				vk::PipelineRasterizationStateCreateFlags(),
				false,	// depth clamp
				false,	// discard all
				vk::PolygonMode::eFill,
				vk::CullModeFlagBits::eBack,
				vk::FrontFace::eClockwise,
				false, 0.f, 0.f, 0.f,	// depth bias
				1.f	// line width
			);
			auto multisampler = vk::PipelineMultisampleStateCreateInfo
			(
				vk::PipelineMultisampleStateCreateFlags(),
				vk::SampleCountFlagBits::e1,
				false,	// enable
				1.f,	// min sample shading
				nullptr,// sample mask
				false,	// alpha to coverage
				false	// alpha to one
			);
			//auto depthStencil = vk::PipelineDepthStencilStateCreateInfo();
			auto colourBlendAttachment = vk::PipelineColorBlendAttachmentState
			(
				false, // enable
				vk::BlendFactor::eOne,	// colour
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd,
				vk::BlendFactor::eOne,	// alpha
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd,
				(
					vk::ColorComponentFlagBits::eR
					| vk::ColorComponentFlagBits::eG
					| vk::ColorComponentFlagBits::eB
					| vk::ColorComponentFlagBits::eA
				)
			);
			auto colourBlending = vk::PipelineColorBlendStateCreateInfo
			(
				vk::PipelineColorBlendStateCreateFlags(),
				false,	// logic operations
				vk::LogicOp::eCopy,
				1, &colourBlendAttachment,
				{0, 0, 0, 0} // blend constants
			);
			//auto dynamicState = vk::PipelineDynamicStateCreateInfo();
			auto gpci = vk::GraphicsPipelineCreateInfo
			(
				vk::PipelineCreateFlags(),
				2, stages.data(),
				&pvisci,
				&piasci,
				nullptr,	// assembly state
				&pvsci,
				&rasterizer,
				&multisampler,
				nullptr,	//&depthStencil,
				&colourBlending,
				nullptr,	//&dynamicState,
				pipelineLayout,
				renderPass,
				0,	// subpass
				vk::Pipeline(), -1 // base pipeline
			);
			auto pipeline = logicalDevice.createGraphicsPipeline(vk::PipelineCache(), gpci);
			logicalDevice.destroyShaderModule(fragShaderCode);
			logicalDevice.destroyShaderModule(vertShaderCode);
			return pipeline;
		}();
		std::cout << "Created graphics pipeline" << std::endl;

		auto framebuffers = [&imageViews, &logicalDevice, &renderPass, &swapExtent]()
		{
			std::vector<vk::Framebuffer> framebuffers;
			for (auto & v : imageViews)	framebuffers.push_back
			(
				logicalDevice.createFramebuffer
				({
					vk::FramebufferCreateFlags(),
					renderPass,
					1, &v,
					swapExtent.width, swapExtent.height,
					1	// layers
				})
			);
			std::cout << "Created " << framebuffers.size() << " framebuffers\n";
			return framebuffers;
		}();

		auto vertexBuffer = [&physicalDevice, &logicalDevice]()
		{
			auto bci = vk::BufferCreateInfo
			(
						vk::BufferCreateFlags(),
						sizeof(vertex_t) * vertices.size(),
						vk::BufferUsageFlagBits::eVertexBuffer,
						vk::SharingMode::eExclusive,
						0, nullptr
					);
					auto buffer = logicalDevice.createBuffer(bci);

			auto requirements = logicalDevice.getBufferMemoryRequirements(buffer);

			auto memoryTypeIndex = [&physicalDevice, &requirements, &buffer](vk::MemoryPropertyFlags propertyMask)
			{
				std::bitset<32>requirementFlags(requirements.memoryTypeBits);
				auto properties = physicalDevice.getMemoryProperties();
				for (uint32_t i = 0; i < properties.memoryTypeCount; ++i) {
					if (requirementFlags[i])
					{
						auto flags = properties.memoryTypes[i].propertyFlags;
						if ((flags & propertyMask) == propertyMask)
						{   // only if propertyMask is entirely contained in the flags
							std::cout << "Found suitible memory type: " << i << ' ' << to_string(flags) << std::endl;
							return i;
						}
					}
				}
				throw std::runtime_error("Failed to find suitable memory type");
			}(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			auto mai = vk::MemoryAllocateInfo
			(
				requirements.size,
				memoryTypeIndex
			);
			auto bufferMemory = logicalDevice.allocateMemory(mai);

			logicalDevice.bindBufferMemory(buffer, bufferMemory, 0); // 0 = offset

			auto memPtr = logicalDevice.mapMemory(bufferMemory, 0, bci.size, vk::MemoryMapFlags() );
			::memcpy(memPtr, vertices.data(), bci.size);
			logicalDevice.unmapMemory(bufferMemory);

			return buffer;
		}();

		auto commandBuffers = [&logicalDevice, &graphicsPipeline, &commandPool, &framebuffers, &renderPass, &swapExtent, &vertexBuffer]()
		{
			auto commandBuffers = logicalDevice.allocateCommandBuffers
			({
				commandPool,
				vk::CommandBufferLevel::ePrimary,
				(uint32_t)framebuffers.size()
			});
			auto f = framebuffers.begin();
			for (auto & b : commandBuffers)
			{
				b.begin
				({
					vk::CommandBufferUsageFlagBits::eSimultaneousUse,
					nullptr //	inheritance info
				});
				constexpr std::array<float,4> clearValues = {0.f, 0.f, 0.f, 1.f};
				auto c = vk::ClearColorValue(clearValues);
				auto cc = vk::ClearValue(c);
				b.beginRenderPass
				(
					{
						renderPass,
						*f,
						vk::Rect2D({0, 0}, swapExtent),
						1, &cc
					},
					vk::SubpassContents::eInline
				);
				b.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
				vk::DeviceSize offset(0);
				b.bindVertexBuffers
				(
					0, 1,
					&vertexBuffer,
					&offset
				);
				b.draw
				(
					vertices.size(),	// vertices
					1,	// instances
					0,	// vertex offset
					0	// instance offset
				);
				b.endRenderPass();
				b.end();
				++f;
			}
			std::cout << "Created " << commandBuffers.size() << " command buffers\n";
			return commandBuffers;
		}();

		// loop time
		mainWindow.show();
		std::cout << "\x1B[32;1mEVERYTHING SET UP, LET'S GO!!!\x1B[m" << std::endl;
		while (not mainWindow.shouldClose())
		{
			uint32_t imageIndex;
			auto result = logicalDevice.acquireNextImageKHR
			(
				swapChain,
				std::numeric_limits<uint64_t>::max(),
				imageAvailable,
				vk::Fence(),
				&imageIndex
			);
			assert(result == vk::Result::eSuccess);

			vk::PipelineStageFlags destStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			auto si = vk::SubmitInfo
			(
				1, &imageAvailable, &destStageMask,
				1, &commandBuffers[imageIndex],
				1, &renderFinished
			);
			result = graphicsQueue.submit(1, &si, vk::Fence() );
			assert(result == vk::Result::eSuccess);
			try {
				result = presentationQueue.presentKHR
				({
					1, &renderFinished,
					1, &swapChain,
					&imageIndex,
					nullptr
				});
				assert(result == vk::Result::eSuccess);
			} catch (vk::OutOfDateKHRError& e)
			{
				std::cout << "\x1B[33;1m" << e.what() << "\x1B[m" << std::endl;
				break;
			}
			mainWindow.swapBuffers();
			//::usleep(100000);
			glfwWaitEvents();
		}
		logicalDevice.waitIdle();

		logicalDevice.destroyBuffer(vertexBuffer);
		for (auto& fb : framebuffers)
			logicalDevice.destroyFramebuffer(fb);
		logicalDevice.destroyRenderPass(renderPass);
		for (auto& v : imageViews)
			logicalDevice.destroyImageView(v);
		logicalDevice.destroySwapchainKHR(swapChain);
		logicalDevice.destroySemaphore(renderFinished);
		logicalDevice.destroySemaphore(imageAvailable);
		logicalDevice.destroyPipelineLayout(pipelineLayout);
		//logicalDevice.destroyQueue(presentationQueue);
		//logicalDevice.destroyQueue(graphicsQueue);
		logicalDevice.destroyCommandPool(commandPool);
		logicalDevice.destroy();
//		physicalDevice.destroy();
		// end
	}
	catch (std::exception &exception)
	{
		std::cerr << exception.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
