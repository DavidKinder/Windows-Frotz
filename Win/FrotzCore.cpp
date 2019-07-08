/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Interface to the Frotz core
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "FrotzApp.h"
#include "FrotzDialogs.h"
#include "FrotzGfx.h"
#include "FrotzSound.h"
#include "FrotzWnd.h"

extern "C"
{
extern char save_name[];
extern char script_name[];
extern char command_name[];
extern char auxilary_name[];

int completion(const zword* buffer, zword* result);
int is_terminator(zword c);
void screen_new_line(void);
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// The single application instance
extern FrotzApp theApp;

// The single output window instance
extern FrotzWnd* theWnd;

/////////////////////////////////////////////////////////////////////////////
// Wrapper classes
/////////////////////////////////////////////////////////////////////////////

class InputTimer
{
public:
  InputTimer(int timeout)
  {
    if (timeout != 0)
      theWnd->SetTimer(FrotzWnd::InputTimer,timeout*100,NULL);
  };

  ~InputTimer()
  {
    if (AfxGetMainWnd())
      theWnd->KillTimer(FrotzWnd::InputTimer);
  };
};

class LineInput
{
public:
  LineInput()
  {
    theApp.SetLineInput(true);
  };

  ~LineInput()
  {
    theApp.SetLineInput(false);
  };
};

/////////////////////////////////////////////////////////////////////////////
// Interface to the Frotz core
/////////////////////////////////////////////////////////////////////////////

/*
 * os_beep
 *
 * Play a beep sound. Ideally, the sound should be high- (number == 1)
 * or low-pitched (number == 2).
 *
 */
extern "C" void os_beep(int number)
{
  theWnd->FlushDisplay();
  ::MessageBeep((number == 1) ? MB_ICONASTERISK : MB_OK);
}

/*
 * os_display_char
 *
 * Display a character of the current font using the current colours and
 * text style. The cursor moves to the next position. Printable codes are
 * all ASCII values from 32 to 126, ISO Latin-1 characters from 160 to
 * 255, ZC_GAP (gap between two sentences) and ZC_INDENT (paragraph
 * indentation), and Unicode characters above 255. The screen should not
 * be scrolled after printing to the bottom right corner.
 *
 */
extern "C" void os_display_char(zchar c)
{
  if (c == ZC_INDENT)
  {
    os_display_char(' ');
    os_display_char(' ');
    os_display_char(' ');
  }
  else if (c == ZC_GAP)
  {
    os_display_char(' ');
    os_display_char(' ');
  }
  else if (theApp.IsValidChar(c))
  {
    theWnd->AddOutput(c,(cwin != 0));
    theApp.SetExitPause(true);
  }
}

/*
 * os_display_string
 *
 * Pass a string of characters to os_display_char.
 *
 */
extern "C" void os_display_string(const zchar *s)
{
  zword c;
  while ((c = *s++) != 0)
  {
    if ((c == ZC_NEW_FONT) || (c == ZC_NEW_STYLE))
    {
      int arg = *s++;
      if (c == ZC_NEW_FONT)
        os_set_font(arg);
      if (c == ZC_NEW_STYLE)
        os_set_text_style(arg);
    }
    else
      os_display_char(c);
  }
}

/*
 * os_erase_area
 *
 * Fill a rectangular area of the screen with the current background
 * colour. Top left coordinates are (1,1). The cursor does not move.
 *
 * The final argument gives the window being changed, -1 if only a
 * portion of a window is being erased, or -2 if the whole screen is
 * being erased.
 *
 */
extern "C" void os_erase_area(int top, int left, int bottom, int right, int win)
{
  theWnd->FlushText();
  theWnd->FillBackground(CRect(left-1,top-1,right,bottom));
}

/*
 * os_fatal
 *
 * Display error message and stop interpreter.
 *
 */
extern "C" void os_fatal(const char *s)
{
  if (theWnd != NULL)
    theWnd->FlushDisplay();

  ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(),s,CResString(IDS_FATAL),MB_ICONERROR|MB_OK);
  exit(0);
}

/*
 * os_font_data
 *
 * Return true if the given font is available. The font can be
 *
 *    TEXT_FONT
 *    PICTURE_FONT
 *    GRAPHICS_FONT
 *    FIXED_WIDTH_FONT
 *
 * The font size should be stored in "height" and "width". If
 * the given font is unavailable then these values must _not_
 * be changed.
 *
 */
extern "C" int os_font_data(int font, int *height, int *width)
{
  switch (font)
  {
  case TEXT_FONT:
  case FIXED_WIDTH_FONT:
  case GRAPHICS_FONT:
    {
      FrotzWnd::TextSettings savedText = theWnd->GetTextSettings();
      theWnd->ApplyTextSettings(FrotzWnd::TextSettings(0,font));

      *height = (zbyte)theWnd->GetFontHeight();
      *width = (zbyte)theWnd->GetCharWidth('0');

      theWnd->ApplyTextSettings(savedText);
      return 1;
    }
  }
  return 0;
}

/*
 * os_read_file_name
 *
 * Return the name of a file. Flag can be one of:
 *
 *    FILE_SAVE      - Save game file
 *    FILE_RESTORE   - Restore game file
 *    FILE_SCRIPT    - Transcript file
 *    FILE_RECORD    - Command file for recording
 *    FILE_PLAYBACK  - Command file for playback
 *    FILE_SAVE_AUX  - Save auxiliary ("preferred settings") file
 *    FILE_LOAD_AUX  - Load auxiliary ("preferred settings") file
 *    FILE_NO_PROMPT - Return file without prompting the user
 *
 * The length of the file name is limited by MAX_FILE_NAME. Ideally
 * an interpreter should open a file requester to ask for the file
 * name. If it is unable to do that then this function should call
 * print_string and read_string to ask for a file name.
 *
 */
extern "C" int os_read_file_name(char *file_name, const char *default_name, int flag)
{
  if (flag == FILE_NO_PROMPT)
  {
    CString path = theApp.GetGameFileName();
    int pos = path.ReverseFind(L'\\');
    if (pos >= 0)
      path.Truncate(pos+1);

    static const char* illegal = "/\\<>:\"|?*";
    while (*default_name != 0)
    {
      if (strchr(illegal,*default_name) == NULL)
        path.AppendChar(*default_name);
      default_name++;
    }

    strncpy(file_name,path,MAX_FILE_NAME);
    return 1;
  }

  theWnd->FlushDisplay();
  theWnd->ResetOverhang();

  bool open = true;
  int filter = 0;
  int title = 0;
  LPCSTR defExt = NULL;

  switch (flag)
  {
    case FILE_SAVE:
      open = false;
      filter = IDS_SAVE_FILTER;
      title = IDS_SAVE_TITLE;
      defExt = "sav";
      break;
    case FILE_RESTORE:
      open = true;
      filter = IDS_SAVE_FILTER;
      title = IDS_RESTORE_TITLE;
      defExt = "sav";
      break;
    case FILE_SCRIPT:
      open = false;
      filter = IDS_SCRIPT_FILTER;
      title = IDS_SCRIPT_TITLE;
      defExt = "log";
      break;
    case FILE_RECORD:
      open = false;
      filter = IDS_RECORD_FILTER;
      title = IDS_RECORD_TITLE;
      defExt = "rec";
      break;
    case FILE_PLAYBACK:
      open = true;
      filter = IDS_RECORD_FILTER;
      title = IDS_PLAYBACK_TITLE;
      defExt = "rec";
      break;
    case FILE_SAVE_AUX:
      open = false;
      filter = IDS_AUX_FILTER;
      title = IDS_SAVE_AUX_TITLE;
      break;
    case FILE_LOAD_AUX:
      open = true;
      filter = IDS_AUX_FILTER;
      title = IDS_LOAD_AUX_TITLE;
      break;
    default:
      return 0;
  }

  SimpleFileDialog dialog(open,defExt,default_name,
    OFN_HIDEREADONLY|OFN_ENABLESIZING|(open ? 0 : OFN_OVERWRITEPROMPT),
    CResString(filter),AfxGetMainWnd());
  CResString titlestr(title);
  dialog.m_ofn.lpstrTitle = titlestr;

  if (dialog.DoModal() == IDOK)
  {
    strncpy(file_name,dialog.GetPathName(),MAX_FILE_NAME);
    return 1;
  }
  return 0;
}

/*
 * os_init_screen
 *
 * Initialise the IO interface. Prepare screen and other devices
 * (mouse, sound card). Set various OS depending story file header
 * entries:
 *
 *     h_config (aka flags 1)
 *     h_flags (aka flags 2)
 *     h_screen_cols (aka screen width in characters)
 *     h_screen_rows (aka screen height in lines)
 *     h_screen_width
 *     h_screen_height
 *     h_font_height (defaults to 1)
 *     h_font_width (defaults to 1)
 *     h_default_foreground
 *     h_default_background
 *     h_interpreter_number
 *     h_interpreter_version
 *     h_user_name (optional; not used by any game)
 *
 * Finally, set reserve_mem to the amount of memory (in bytes) that
 * should not be used for multiple undo and reserved for later use.
 *
 */
extern "C" void os_init_screen(void)
{
  theApp.CreateMainWindow();

  // Look for a Blorb resource file
  theApp.SetBlorbFile();

  // Set the configuration
  if (h_version == V3)
  {
    h_config |= CONFIG_SPLITSCREEN;
    h_config |= CONFIG_PROPORTIONAL;
    if (theApp.IsTandyBitSet())
      h_config |= CONFIG_TANDY;
    else
      h_config &= ~CONFIG_TANDY;
  }
  if (h_version >= V4)
  {
    h_config |= CONFIG_BOLDFACE;
    h_config |= CONFIG_EMPHASIS;
    h_config |= CONFIG_FIXED;
    h_config |= CONFIG_TIMEDINPUT;
  }
  if (h_version >= V5)
    h_config |= CONFIG_COLOUR;
  if (h_version == V6)
  {
    if (theApp.GotBlorbFile())
    {
      h_config |= CONFIG_PICTURES;
      h_config |= CONFIG_SOUND;
    }
  }
  theApp.CopyUsername();

  h_interpreter_version = 'F';
  if (h_version == V6)
  {
    h_default_foreground = theApp.GetColourIndex(theApp.GetDefaultColour(true));
    h_default_background = theApp.GetColourIndex(theApp.GetDefaultColour(false));
  }
  else
  {
    h_default_foreground = 1;
    h_default_background = 1;
  }

  theWnd->ApplyTextSettings(
    FrotzWnd::TextSettings(0,FIXED_WIDTH_FONT));
  h_font_width = (zbyte)theWnd->GetCharWidth('0');
  h_font_height = (zbyte)theWnd->GetFontHeight();

  CRect wndSize;
  theWnd->GetClientRect(wndSize);
  h_screen_width = (zword)wndSize.Width();
  h_screen_height = (zword)wndSize.Height();
  h_screen_cols = (zbyte)(h_screen_width / h_font_width);
  h_screen_rows = (zbyte)(h_screen_height / h_font_height);

  // Check for sound
  if ((h_version == V3) && (h_flags & OLD_SOUND_FLAG))
  {
    if (!theApp.GotBlorbFile() || !FrotzSound::Init(theWnd))
      h_flags &= ~OLD_SOUND_FLAG;
  }
  else if ((h_version >= V4) && (h_flags & SOUND_FLAG))
  {
    if (!theApp.GotBlorbFile() || !FrotzSound::Init(theWnd))
      h_flags &= ~SOUND_FLAG;
  }

  if (h_version >= V5)
  {
    zword mask = 0;
    if (h_version == V6)
      mask |= TRANSPARENT_FLAG;

    // Mask out any unsupported bits in the extended flags
    hx_flags &= mask;

    hx_fore_colour = FrotzApp::TrueToRGB5(theApp.GetDefaultColour(true));
    hx_back_colour = FrotzApp::TrueToRGB5(theApp.GetDefaultColour(false));
  }
}

/*
 * os_more_prompt
 *
 * Display a MORE prompt, wait for a keypress and remove the MORE
 * prompt from the screen.
 *
 */
extern "C" void os_more_prompt(void)
{
  if (theApp.GetShowMorePrompts())
  {
    theWnd->FlushText();
    theWnd->ResetOverhang();

    // Save the current text position
    CPoint point = theWnd->GetTextPoint();

    // Show a [More] prompt
    theWnd->WriteText(CResString(IDS_MORE));
    theWnd->DrawCursor(true);
    theWnd->FlushDisplay();
    theApp.SpeakText();

    // Wait for a key press
    FrotzWnd::Input input;
    while (true)
    {
      theWnd->WaitForInput();
      while (theWnd->GetNextInput(input))
      {
        switch (input.type)
        {
        case FrotzWnd::Input::ZcodeKey:
          // Remove the [More] prompt
          theWnd->FillBackground(CRect(point.x,point.y,
            theWnd->GetTextPoint().x,point.y+theWnd->GetFontHeight()));
          theWnd->DrawCursor(false);

          // Restore the current text position
          theWnd->SetTextPoint(point);
          return;
        case FrotzWnd::Input::Reset:
          point.y = theWnd->GetTextPoint().y;
          break;
        case FrotzWnd::Input::CheckRestart:
          theApp.CheckRestart();
          break;
        }
      }
    }
  }
}

/*
 * os_process_arguments
 *
 * Handle command line switches. Some variables may be set to activate
 * special features of Frotz:
 *
 *     option_attribute_assignment
 *     option_attribute_testing
 *     option_context_lines
 *     option_object_locating
 *     option_object_movement
 *     option_left_margin
 *     option_right_margin
 *     option_ignore_errors
 *     option_piracy
 *     option_undo_slots
 *     option_expand_abbreviations
 *     option_script_cols
 *
 * The global pointer "story_name" is set to the story file name.
 *
 */
extern "C" void os_process_arguments(int argc, char *argv[])
{
  // Ask the user for a game filename
  story_name = (char*)theApp.GetGameFileName();
  if (story_name == NULL)
    exit(0);

  // Set default filenames
  CString filename = story_name;
  if (filename.ReverseFind('.') > 0)
  {
    filename = filename.Left(filename.ReverseFind('.'));

    strcpy(save_name,filename);
    strcpy(script_name,filename);
    strcpy(command_name,filename);
    strcpy(auxilary_name,filename);

    strcat(save_name,".sav");
    strcat(script_name,".log");
    strcat(command_name,".rec");
    strcat(auxilary_name,".aux");
  }
}

/*
 * os_read_line
 *
 * Read a line of input from the keyboard into a buffer. The buffer
 * may already be primed with some text. In this case, the "initial"
 * text is already displayed on the screen. After the input action
 * is complete, the function returns with the terminating key value.
 * The length of the input should not exceed "max" characters plus
 * an extra 0 terminator.
 *
 * Terminating keys are the return key (13) and all function keys
 * (see the Specification of the Z-machine) which are accepted by
 * the is_terminator function. Mouse clicks behave like function
 * keys except that the mouse position is stored in global variables
 * "mouse_x" and "mouse_y" (top left coordinates are (1,1)).
 *
 * Furthermore, Frotz introduces some special terminating keys:
 *
 *     ZC_HKEY_PLAYBACK (Alt-P)
 *     ZC_HKEY_RECORD (Alt-R)
 *     ZC_HKEY_SEED (Alt-S)
 *     ZC_HKEY_UNDO (Alt-U)
 *     ZC_HKEY_RESTART (Alt-N, "new game")
 *     ZC_HKEY_QUIT (Alt-X, "exit game")
 *     ZC_HKEY_DEBUG (Alt-D)
 *     ZC_HKEY_HELP (Alt-H)
 *
 * If the timeout argument is not zero, the input gets interrupted
 * after timeout/10 seconds (and the return value is 0).
 *
 * The complete input line including the cursor must fit in "width"
 * screen units.
 *
 * The function may be called once again to continue after timeouts,
 * misplaced mouse clicks or hot keys. In this case the "continued"
 * flag will be set. This information can be useful if the interface
 * implements input line history.
 *
 * The screen is not scrolled after the return key was pressed. The
 * cursor is at the end of the input line when the function returns.
 *
 * Since Frotz 2.2 the helper function "completion" can be called
 * to implement word completion (similar to tcsh under Unix).
 *
 */
extern "C" zchar os_read_line(int max, zchar *buf, int timeout, int width, int continued)
{
  static int prev_pos = 0;
  static int prev_history = 0;

  LineInput line;
  theWnd->FlushText();
  theWnd->ResetOverhang();
  theWnd->UpdateMenus();
  theApp.SpeakText();
  theWnd->RecaseInput(buf);

  // Find the editing position
  int pos = 0;
  if (continued)
  {
    if (prev_pos <= (int)wcslen((LPCWSTR)buf))
      pos = prev_pos;
  }
  else
    pos = wcslen((LPCWSTR)buf);

  // Find the input history position
  int history = 0;
  if (continued)
    history = prev_history;

  // Draw the input line
  CPoint point = theWnd->GetTextPoint();
  point.x -= theWnd->GetTextWidth(buf,wcslen((LPCWSTR)buf));
  theWnd->DrawInput(buf,pos,point,width,true);

  InputTimer timer(timeout);
  while (true)
  {
    // Get the next input
    theWnd->WaitForInput();

    FrotzWnd::Input input;
    while (theWnd->GetNextInput(input))
    {
      switch (input.type)
      {
      case FrotzWnd::Input::ZcodeKey:
        {
          zword c = (zword)(input.in);
          if (c == ZC_BACKSPACE)
          {
            // Delete the character to the left of the cursor
            if (pos > 0)
            {
              memmove(buf+pos-1,buf+pos,sizeof(zword)*(wcslen((LPCWSTR)buf)-pos+1));
              pos--;
              theWnd->DrawInput(buf,pos,point,width,true);
            }
          }
          else if ((c == ZC_ARROW_LEFT) && (input.modify == false))
          {
            // Move the cursor left
            if (pos > 0)
              pos--;
            theWnd->DrawInput(buf,pos,point,width,true);
          }
          else if ((c == ZC_ARROW_RIGHT) && (input.modify == false))
          {
            // Move the cursor right
            if (pos < (int)wcslen((LPCWSTR)buf))
              pos++;
            theWnd->DrawInput(buf,pos,point,width,true);
          }
          else if ((c == ZC_ARROW_UP) && (input.modify == false))
          {
            // Move up through the command history
            if (theWnd->InputFromHistory(history))
            {
              history++;
              *buf = 0;
              pos = 0;
            }
            theWnd->DrawInput(buf,pos,point,width,true);
          }
          else if ((c == ZC_ARROW_DOWN) && (input.modify == false))
          {
            // Move down through the command history
            if (history > 1)
            {
              if (theWnd->InputFromHistory(history-2))
              {
                history--;
                *buf = 0;
                pos = 0;
              }
            }
            else if (history == 1)
            {
              history = 0;
              *buf = 0;
              pos = 0;
            }
            theWnd->DrawInput(buf,pos,point,width,true);
          }
          else if (is_terminator(c))
          {
            // Terminate the current input
            theApp.SetExitPause(false);
            theWnd->DrawInput(buf,pos,point,width,false);

            if ((c == ZC_SINGLE_CLICK) || (c == ZC_DOUBLE_CLICK))
            {
              mouse_x = input.mousex+1;
              mouse_y = input.mousey+1;
            }
            else if (c == ZC_RETURN)
              theWnd->AddToInputHistory(buf);

            theWnd->SetLastInput(buf);
            prev_pos = pos;
            prev_history = history;
            return c;
          }
          else if (theApp.IsValidChar(c))
          {
            // Add a valid character to the input line
            if ((int)wcslen((LPCWSTR)buf) < max)
            {
              // Get the width of the new input line
              int len = theWnd->GetTextWidth(buf,wcslen((LPCWSTR)buf));
              len += theWnd->GetCharWidth(c);
              len += theWnd->GetCharWidth('0');

              // Only allow if the width limit is not exceeded
              if (len <= width)
              {
                memmove(buf+pos+1,buf+pos,sizeof(zword)*(wcslen((LPCWSTR)buf)-pos+1));
                *(buf+pos) = c;
                pos++;
                theWnd->DrawInput(buf,pos,point,width,true);
              }
            }
          }
        }
        break;
      case FrotzWnd::Input::VirtualKey:
        {
          switch (input.in)
          {
          case VK_DELETE:
            // Delete the character under the cursor
            if (pos < (int)wcslen((LPCWSTR)buf))
            {
              memmove(buf+pos,buf+pos+1,sizeof(zword)*(wcslen((LPCWSTR)buf)-pos));
              theWnd->DrawInput(buf,pos,point,width,true);
            }
            break;
          case VK_HOME:
            // Move the cursor to the start of the line
            pos = 0;
            theWnd->DrawInput(buf,pos,point,width,true);
            break;
          case VK_END:
            // Move the cursor to the end of the line
            pos = wcslen((LPCWSTR)buf);
            theWnd->DrawInput(buf,pos,point,width,true);
            break;
          case VK_TAB:
            if (pos == (int)wcslen((LPCWSTR)buf))
            {
              zword extension[10];
              completion(buf,extension);

              // Add the completion to the input stream
              for (zword* s = extension; *s != 0; s++)
                theWnd->InputUnicode(*s);
            }
            break;
          }
        }
        break;
      case FrotzWnd::Input::Reset:
        point.y = theWnd->GetTextPoint().y;
        theWnd->EraseLastInputRect(point);
        theWnd->DrawInput(buf,pos,point,width,true);
        break;
      case FrotzWnd::Input::KillLine:
        *(buf+pos) = 0;
        theWnd->DrawInput(buf,pos,point,width,true);
        break;
      case FrotzWnd::Input::RuboutWord:
        // Find the start of the next word to the left of the cursor
        {
          int c = 0;
          bool inword = false;
          while (c < pos)
          {
            if (inword)
            {
              if (*(buf+pos-c-1) == ' ')
                break;
            }
            else
            {
              if (*(buf+pos-c-1) != ' ')
                inword = true;
            }
            c++;
          }

          // Delete the word
          memmove(buf+pos-c,buf+pos,sizeof(zword)*(wcslen((LPCWSTR)buf)-pos+c));
          pos -= c;
          theWnd->DrawInput(buf,pos,point,width,true);
        }
        break;
      case FrotzWnd::Input::CheckRestart:
        theApp.CheckRestart();
        break;
      }
    }
  }
  return 0;
}

/*
 * os_read_key
 *
 * Read a single character from the keyboard (or a mouse click) and
 * return it. Input aborts after timeout/10 seconds.
 *
 */
extern "C" zchar os_read_key(int timeout, int cursor)
{
  theWnd->FlushText();
  theWnd->ResetOverhang();
  theWnd->UpdateMenus();
  if (cursor)
    theWnd->DrawCursor(true);
  theWnd->FlushDisplay();
  theApp.SpeakText();

  InputTimer timer(timeout);
  FrotzWnd::Input input;
  while (true)
  {
    // Get the next input
    theWnd->WaitForInput();
    while (theWnd->GetNextInput(input))
    {
      switch (input.type)
      {
      case FrotzWnd::Input::ZcodeKey:
        theApp.SetExitPause(false);

        if ((input.in == ZC_SINGLE_CLICK) || (input.in == ZC_DOUBLE_CLICK))
        {
          mouse_x = input.mousex+1;
          mouse_y = input.mousey+1;
        }

        if (cursor)
          theWnd->DrawCursor(false);
        return (zword)(input.in);

      case FrotzWnd::Input::CheckRestart:
        theApp.CheckRestart();
        break;
      }
    }
  }
}

/*
 * os_read_mouse
 *
 * Store the mouse position in the global variables "mouse_x" and
 * "mouse_y", the code of the last clicked menu in "menu_selected"
 * and return the mouse buttons currently pressed.
 *
 */
zword os_read_mouse(void)
{
  // Get the mouse position
  POINT pos;
  if (::GetCursorPos(&pos))
  {
    theWnd->ScreenToClient(&pos);
    mouse_x = pos.x+1;
    mouse_y = pos.y+1;
  }

  // Get the last selected menu item
  menu_selected = theWnd->GetMenuClick();

  // Get the mouse buttons
  zword btn = 0;
  if (::GetKeyState(VK_LBUTTON) & 0x8000)
    btn |= 1;
  if (::GetKeyState(VK_RBUTTON) & 0x8000)
    btn |= 2;
  if (::GetKeyState(VK_MBUTTON) & 0x8000)
    btn |= 4;
  return btn;
}

/*
 * os_menu
 *
 * Add to or remove a menu item. Action can be:
 *     MENU_NEW    - Add a new menu with the given title
 *     MENU_ADD    - Add a new menu item with the given text
 *     MENU_REMOVE - Remove the menu at the given index
 *
 */
void os_menu(int action, int menu, const zword * text)
{
  switch (action)
  {
  case MENU_NEW:
    theWnd->AddNewMenu(menu,text);
    break;
  case MENU_ADD:
    theWnd->AddMenuItem(menu,text);
    break;
  case MENU_REMOVE:
    theWnd->RemoveMenu(menu);
    break;
  }
}

/*
 * os_reset_screen
 *
 * Reset the screen before the program ends.
 *
 */
extern "C" void os_reset_screen(void)
{
  theWnd->FlushDisplay();
  theWnd->ResetOverhang();

  if (theApp.GetExitPause())
  {
    os_set_font(TEXT_FONT);
    os_set_text_style(0);
    screen_new_line();

    CResString hit(IDS_HIT_KEY_EXIT);
    for (int i = 0; i < hit.GetLength(); i++)
      os_display_char((unsigned char)hit[i]);
    os_read_key(0,1);
  }
}

/*
 * os_scroll_area
 *
 * Scroll a rectangular area of the screen up (units > 0) or down
 * (units < 0) and fill the empty space with the current background
 * colour. Top left coordinates are (1,1). The cursor stays put.
 *
 */
extern "C" void os_scroll_area(int top, int left, int bottom, int right, int units)
{
  theWnd->FlushText();
  theWnd->ResetOverhang();

  theWnd->Scroll(CRect(left-1,top-1,right,bottom),units);
  if (units > 0)
    theWnd->FillBackground(CRect(left-1,bottom-units,right,bottom));
  else
    theWnd->FillBackground(CRect(left-1,top-1,right,top+units-1));

  if (theApp.GetFastScrolling() == false)
    theWnd->FlushDisplay();
}

/*
 * os_set_colour
 *
 * Set the foreground and background colours which can be:
 *
 *     1
 *     BLACK_COLOUR
 *     RED_COLOUR
 *     GREEN_COLOUR
 *     YELLOW_COLOUR
 *     BLUE_COLOUR
 *     MAGENTA_COLOUR
 *     CYAN_COLOUR
 *     WHITE_COLOUR
 *     TRANSPARENT_COLOUR
 *
 *     Amiga only:
 *
 *     LIGHTGREY_COLOUR
 *     MEDIUMGREY_COLOUR
 *     DARKGREY_COLOUR
 *
 * There may be more colours in the range from 16 to 255; see the
 * remarks about os_peek_colour.
 *
 */
extern "C" void os_set_colour(int new_foreground, int new_background)
{
  theWnd->FlushText();
  theWnd->ResetOverhang();

  if (new_foreground == 1)
    theWnd->GetTextSettings().fore = theApp.GetDefaultColour(true);
  else if (new_foreground < 256)
    theWnd->GetTextSettings().fore = theApp.GetColour(new_foreground);
  theWnd->GetTextSettings().foreDefault = (new_foreground == 1);

  if (new_background == 1)
    theWnd->GetTextSettings().back = theApp.GetDefaultColour(false);
  else if (new_background < 256)
    theWnd->GetTextSettings().back = theApp.GetColour(new_background);
  theWnd->GetTextSettings().backDefault = (new_background == 1);
  theWnd->GetTextSettings().backTransparent = (new_background == 15);

  theWnd->ApplyTextSettings();
}

/*
 * os_from_true_cursor
 *
 * Given a true colour, return an appropriate colour index.
 *
 */
extern "C" int os_from_true_colour(zword colour)
{
  return theApp.GetColourIndex(FrotzApp::RGB5ToTrue(colour));
}

/*
 * os_to_true_cursor
 *
 * Given a colour index, return the appropriate true colour.
 *
 */
extern "C" zword os_to_true_colour(int index)
{
  return FrotzApp::TrueToRGB5(theApp.GetColour(index));
}

/*
 * os_set_cursor
 *
 * Place the text cursor at the given coordinates. Top left is (1,1).
 *
 */
extern "C" void os_set_cursor(int row, int col)
{
  theWnd->FlushText();
  theWnd->ResetOverhang();
  theWnd->SetTextPoint(CPoint(col-1,row-1));
}

/*
 * os_set_font
 *
 * Set the font for text output. The interpreter takes care not to
 * choose fonts which aren't supported by the interface.
 *
 */
extern "C" void os_set_font(int new_font)
{
  theWnd->FlushText();
  theWnd->GetTextSettings().font = new_font;
  theWnd->ApplyTextSettings();
}

/*
 * os_set_text_style
 *
 * Set the current text style. Following flags can be set:
 *
 *     REVERSE_STYLE
 *     BOLDFACE_STYLE
 *     EMPHASIS_STYLE (aka underline aka italics)
 *     FIXED_WIDTH_STYLE
 *
 */
extern "C" void os_set_text_style(int new_style)
{
  theWnd->FlushText();
  theWnd->GetTextSettings().style = new_style;
  theWnd->ApplyTextSettings();
}

/*
 * os_string_width
 *
 * Calculate the length of a word in screen units. Apart from letters,
 * the word may contain special codes:
 *
 *    ZC_NEW_STYLE - next character is a new text style
 *    ZC_NEW_FONT  - next character is a new font
 *
 */
extern "C" int os_string_width(const zchar *s)
{
  // Look for style or font changes, or indents
  bool changes = false;
  const zword* s1 = NULL;
  for (s1 = s; *s1 != 0; s1++)
  {
    if ((*s1 == ZC_NEW_STYLE) || (*s1 == ZC_NEW_FONT))
    {
      changes = true;
      break;
    }
    if ((*s1 == ZC_INDENT) || (*s1 == ZC_GAP))
    {
      changes = true;
      break;
    }
  }

  int width = 0;

  // If there are no changes, just get the width of the string
  if (changes == false)
    width = theWnd->GetTextWidth(s,wcslen((LPCWSTR)s));
  else
  {
    // Get the width of a space
    int spWidth = os_char_width(' ');

    // Save the current text settings
    FrotzWnd::TextSettings savedText = theWnd->GetTextSettings();

    // Work out the length of each section of the text
    s1 = s;
    const zword* s2 = s;
    bool done = false;
    while (done == false)
    {
      // Is this the end of a section of text?
      switch (*s2)
      {
      case ZC_NEW_STYLE:
        if (s2 > s1)
          width += theWnd->GetTextWidth(s1,s2-s1);
        s2++;
        os_set_text_style(*s2);
        s1 = s2+1;
        break;
      case ZC_NEW_FONT:
        if (s2 > s1)
          width += theWnd->GetTextWidth(s1,s2-s1);
        s2++;
        os_set_font(*s2);
        s1 = s2+1;
        break;
      case ZC_INDENT:
        if (s2 > s1)
          width += theWnd->GetTextWidth(s1,s2-s1);
        width += spWidth*3;
        s1++;
        break;
      case ZC_GAP:
        if (s2 > s1)
          width += theWnd->GetTextWidth(s1,s2-s1);
        width += spWidth*2;
        s1++;
        break;
      case 0:
        if (s2 > s1)
          width += theWnd->GetTextWidth(s1,s2-s1);
        done = true;
        break;
      }
      s2++;
    }

    // Restore the current text settings
    theWnd->ApplyTextSettings(savedText);
  }
  return width;
}

/*
 * os_char_width
 *
 * Return the length of the character in screen units.
 *
 */
extern "C" int os_char_width(zchar c)
{
  return theWnd->GetCharWidth(c);
}

/*
 * os_check_unicode
 *
 * Return with bit 0 set if the Unicode character can be
 * displayed, and bit 1 if it can be input.
 * 
 *
 */
extern "C" int os_check_unicode(int font, zchar c)
{
  return theWnd->HasGlyph(font,c) ? 3 : 2;
}

/*
 * os_peek_colour
 *
 * Return the colour of the screen unit below the cursor. (If the
 * interface uses a text mode, it may return the background colour
 * of the character at the cursor position instead.) This is used
 * when text is printed on top of pictures. Note that this coulor
 * need not be in the standard set of Z-machine colours. To handle
 * this situation, Frotz entends the colour scheme: Colours above
 * 15 (and below 256) may be used by the interface to refer to non
 * standard colours. Of course, os_set_colour must be able to deal
 * with these colours.
 *
 */
extern "C" int os_peek_colour(void)
{
  theWnd->FlushText();

  COLORREF colour = theWnd->GetPixel(theWnd->GetTextPoint());
  return theApp.GetColourIndex(colour);
}

/*
 * os_picture_data
 *
 * Return true if the given picture is available. If so, store the
 * picture width and height in the appropriate variables. Picture
 * number 0 is a special case: Write the highest legal picture number
 * and the picture file release number into the height and width
 * variables respectively when this picture number is asked for.
 *
 */
extern "C" int os_picture_data(int picture, int *height, int *width)
{
  if (theApp.GotBlorbFile())
  {
    if (picture == 0)
    {
      bb_count_resources(theApp.GetBlorbMap(),bb_ID_Pict,height,NULL,NULL);
      *width = bb_get_release_num(theApp.GetBlorbMap());
      return 1;
    }
    else
    {
      FrotzGfx* gfx = FrotzGfx::Get(picture,theApp.GetBlorbMap(),false);
      if (gfx != NULL)
      {
        CSize size = theWnd->GetGraphicSize(gfx);
        *height = size.cy;
        *width = size.cx;
        return 1;
      }
    }
  }
  *height = 0;
  *width = 0;
  return 0;
}

/*
 * os_draw_picture
 *
 * Display a picture at the given coordinates.
 *
 */
extern "C" void os_draw_picture(int picture, int y, int x)
{
  theWnd->FlushText();

  if (theApp.GotBlorbFile())
  {
    FrotzGfx* gfx = FrotzGfx::Get(picture,theApp.GetBlorbMap(),false);
    if (gfx != NULL)
    {
      CPoint point((short)x-1,(short)y-1);
      theWnd->DrawGraphic(gfx,point);
    }
  }
}

/*
 * os_random_seed
 *
 * Return an appropriate random seed value in the range from 0 to
 * 32767, possibly by using the current system time.
 *
 */
extern "C" int os_random_seed(void)
{
  return ::GetTickCount() & 32767;
}

/*
 * os_restart_game
 *
 * This routine allows the interface to interfere with the process of
 * restarting a game at various stages:
 *
 *     RESTART_BEGIN - restart has just begun
 *     RESTART_WPROP_SET - window properties have been initialised
 *     RESTART_END - restart is complete
 *
 */
extern "C" void os_restart_game(int stage)
{
  // Show Beyond Zork's title screen
  if ((stage == RESTART_BEGIN) && (story_id == BEYOND_ZORK))
  {
    int w,h;
    if (os_picture_data(1,&h,&w))
    {
      CRect size;
      theWnd->GetClientRect(size);
      theWnd->FillSolid(size,RGB(0,0,0));

      os_draw_picture(1,1,1);
      os_read_key(0,0);
    }
  }
}

/*
 * os_path_open
 *
 * Open a file in the current directory.
 *
 */
extern "C" FILE *os_path_open(const char *name, const char *mode, long *size)
{
  FILE* file = fopen(name,mode);
  if (file != NULL)
    theApp.SetBlorbZCode(file,size);
  return file;
}

/*
 * os_finish_with_sample
 *
 * Remove the current sample from memory (if any).
 *
 */
extern "C" void os_finish_with_sample(int number)
{
  FrotzSound::Stop(number);
}

/*
 * os_prepare_sample
 *
 * Load the given sample from the disk.
 *
 */
extern "C" void os_prepare_sample(int number)
{
}

/*
 * os_start_sample
 *
 * Play the given sample at the given volume (ranging from 1 to 8 and
 * 255 meaning a default volume). The sound is played once or several
 * times in the background (255 meaning forever). The end_of_sound
 * function is called as soon as the sound finishes, passing in the
 * eos argument.
 *
 */
extern "C" void os_start_sample(int number, int volume, int repeats, zword eos)
{
  if (theApp.GotBlorbFile())
  {
    if (volume == 255)
      volume = 8;

    if (repeats == 0)
      repeats = 1;
    else if (repeats == 255)
      repeats = -1;

    FrotzSound::Play(number,theApp.GetBlorbMap(),repeats,volume,eos);
  }
  else if ((story_id == SHERLOCK) || (story_id == LURKING_HORROR))
    FrotzSound::MsgInfocomBlorb();
}

/*
 * os_stop_sample
 *
 * Turn off the current sample.
 *
 */
extern "C" void os_stop_sample(int number)
{
  FrotzSound::Stop(number);
}

/*
 * os_scrollback_char
 *
 * Write a character to the scrollback buffer.
 *
 */
extern "C" void  os_scrollback_char(zword c)
{
  theApp.ScrollbackChar(c);
}

/*
 * os_scrollback_erase
 *
 * Remove characters from the scrollback buffer.
 *
 */
extern "C" void os_scrollback_erase (int erase)
{
  theApp.ScrollbackRemove(erase);
}

/*
 * os_tick
 *
 * Called after each opcode.
 *
 */
extern "C" void os_tick (void)
{
  static int count = 0;

  // Check for completed sounds
  if (++count > 1000)
  {
    count = 0;
    FrotzSound::OnNotify();
  }

  // Check for restart
  theApp.CheckRestart();
}

/*
 * os_buffer_screen
 *
 * Set the screen buffering mode, and return the previous mode.
 * Possible values for mode are:
 *
 *     0 - update the display to reflect changes when possible
 *     1 - do not update the display
 *    -1 - redraw the screen, do not change the mode
 *
 */
extern "C" int os_buffer_screen (int mode)
{
  if (mode == -1)
    theWnd->FlushDisplay();
  return 0;
}

/*
 * os_wrap_window
 *
 * Return non-zero if the window should have text wrapped.
 *
 */
extern "C" int os_wrap_window (int win)
{
  return 1;
}

/*
 * os_window_height
 *
 * Called when the height of a window is changed.
 *
 */
extern "C" void os_window_height (int win, int height)
{
}
