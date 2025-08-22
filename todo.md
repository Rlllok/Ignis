#TODO
-[?] Render in Texture and then copy to Swapchain
    I think, this is not how it should work. Not through copy(blit). I tried but it's not working.
    The reason is there is no garanty that swapchain image can supprot transfer. Or, maybe, there is
    synchronization problem that I doesn't know how to solve.
    But it can be posible to just add pass where we draw texture on fullscreen quad;
    -[x] Copy Texture
-[] Mouse Picking
    - [x] Draw Texture filled with EntityID
    - [x] Read Texture data from CPU
    - [] Add selected Entity and do something with it to test how it works
        (For example, rotate if R is down)
