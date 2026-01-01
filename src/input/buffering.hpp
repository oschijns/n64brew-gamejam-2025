#pragma once


#include <cstdint>
#include <libdragon.h>


namespace jam::input
{
    // MARK: Button Mask

    /// @brief Enumerate the bits to check for each button
    enum class ButtonMask : unsigned
    {
        NONE    = 0,
        A       = 0x0001,
        B       = 0x0002,
        Z       = 0x0004,
        START   = 0x0008,
        D_UP    = 0x0010,
        D_DOWN  = 0x0020,
        D_LEFT  = 0x0040,
        D_RIGHT = 0x0080,
        Y       = 0x0100,
        X       = 0x0200,
        L       = 0x0400,
        R       = 0x0800,
        C_UP    = 0x1000,
        C_DOWN  = 0x2000,
        C_LEFT  = 0x4000,
        C_RIGHT = 0x8000,
    };


    // MARK: Buffering

    class Buffering
    {
    protected:
        /// @brief Keep track of the last time the button was pressed
        uint64_t timestamp_buffer = 0;

        /// @brief Keep track of the last time the action to trigger was allowed to happen
        uint64_t timestamp_coyote = 0;

        /// @brief Function to call when the action must be performed
        void (*action_callback)() = nullptr;

        /// @brief Specify the button to check
        ButtonMask mask = ButtonMask::NONE;

        /// @brief Delay used to read this button's state in ticks
        /// @note This should roughly correspond to a human's reaction time ~200 ms
        uint32_t ticks_delay = 0;

        /// @brief Is the button being currently pressed?
        bool pressed = false;

        /// @brief Are we in a state where the action can be performed?
        bool allow_action = false;

    public:
        /// @brief Default constructor
        inline Buffering(
            uint32_t delay_ms, 
            ButtonMask mask_,
            void (*action_callback_)()
        ):
            action_callback(action_callback_),
            mask(mask_),
            ticks_delay(TICKS_FROM_MS(delay_ms))
        {}

        /// @brief Default destructor
        inline ~Buffering() = default;

    public:

        /// @brief Specify if the button is currently being pressed
        inline bool is_pressed() const { return pressed; }

        /// @brief Reset the timers
        inline void reset_timers()
        {
            timestamp_buffer = 0;
            timestamp_coyote = 0;
        }

        /// @brief Read a buttons state to feed this buffering buffer
        /// @param btns buttons state to read from
        /// @note `btns` is provided by `joypad_get_buttons_pressed`
        void read_button(joypad_buttons_t btns);

        /// @brief Update the allowing state
        /// @param new_state New state to set in this buffering input
        void set_new_state(bool new_state);
    };
}