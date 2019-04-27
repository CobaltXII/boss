// A raw interface to a SDL_Window*.
class video_interface {
private:
	// SDL internals.
	SDL_Window* sdl_window = NULL;
	SDL_Renderer* sdl_renderer = NULL;
	SDL_Texture* sdl_texture = NULL;
	
public:
	// Dimensions of the SDL_Window*.
	int x_res;
	int y_res;
	
	// Video memory pointer.
	Uint32* video = NULL;

	// Quit.
	void quit() {
		// Free the video memory.
		free(video);
		// Destroy all SDL objects.
		SDL_DestroyTexture(sdl_texture);
		SDL_DestroyRenderer(sdl_renderer);
		SDL_DestroyWindow(sdl_window);
		// Quit SDL.
		SDL_Quit();
		// Exit.
		exit(0);
	}
	
	// Set the value of a certain pixel of the video memory. No bounds
	// checking is done in this function.
	inline void set(int x, int y, Uint32 pixel) {
		video[y * x_res + x] = pixel;
	}

	// Set the value of a certain pixel of the video memory. Bounds checking is
	// done in this function.
	inline void set_safe(int x, int y, Uint32 pixel) {
		if (x >= 0 && x < x_res)
		if (y >= 0 && y < y_res) {
			video[y * x_res + x] = pixel;
		}
	}
	
	// Default constructor.
	video_interface(const char* title,
					int x_res,
					int y_res)
	{
		this->x_res = x_res;
		this->y_res = y_res;
		
		// Create the SDL_Window*.
		sdl_window = SDL_CreateWindow(
			title,
			// Let the operating system pick the window's position.
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			x_res,
			y_res,
			// This flag will cause pixels to be rendered correctly (without
			// linear interpolation) on high-DPI displays.
			SDL_WINDOW_ALLOW_HIGHDPI
		);
		
		if (!sdl_window) {
			barf("Could not create a SDL_Window*.");
		}
		
		// Create the SDL_Renderer*.
		sdl_renderer = SDL_CreateRenderer(
			sdl_window,
			-1,
			// Some systems may have a GPU-accelerated renderer. On other
			// systems the renderer will fall back to software.
			SDL_RENDERER_ACCELERATED
		);
		
		if (!sdl_renderer) {
			barf("Could not create a SDL_Renderer*.");
		}
		
		// Create the SDL_Texture*.
		sdl_texture = SDL_CreateTexture(
			sdl_renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			x_res,
			y_res
		);
		
		if (!sdl_texture) {
			barf("Could not create a SDL_Texture*.");
		}
		
		// Allocate video memory.
		video = (Uint32*)malloc(x_res * y_res * sizeof(Uint32));
		
		if (!video)
			barf("Could not allocate video memory.");
	}
	
	// Video output function.
	void push() {
		// Update the SDL_Texture*.
		SDL_UpdateTexture(sdl_texture, NULL, video, x_res * sizeof(Uint32));
		// Copy the SDL_Texture* to the SDL_Renderer*.
		SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
		// Update the SDL_Renderer*.
		SDL_RenderPresent(sdl_renderer);
	}

	// Save the video buffer as a .bmp file.
	void save_bmp(std::string filename) {
		// Create a SDL_Surface* from the video memory.
		SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
			video,
			x_res,
			y_res,
			32,
			x_res * 4,
			0, 0, 0, 0
		);
		// Save the SDL_Surface*.
		SDL_SaveBMP(surface, filename.c_str());
		// Free the SDL_Surface*.
		SDL_FreeSurface(surface);
	}
};