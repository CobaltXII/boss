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
};