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
};