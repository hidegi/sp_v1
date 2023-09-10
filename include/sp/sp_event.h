#ifndef SP_EVENT_H_INCLUDED
#define SP_EVENT_H_INCLUDED

namespace sp
{
    enum SP_SystemEvent : long int
	{
			None	= 0,
			WindowMove,
			WindowSize,
			WindowClose,
			WindowRefresh,
			WindowFocus,
			WindowIconify,
			WindowMaximize,
			WindowFramebufferSize,
			WindowContentScale,
			EventMouseButton,
			EventCursorPos,
			EventCursorEnter,
			EventScroll,
			EventKey,
			EventChar,
			EventCharMods,
			EventDrop,
			EventUpdate,
			UserEvent,
			SystemEventCount = EventUpdate + 1 //19..
	};

	struct SP_Detail
	{
		struct SP_KeyStruct
		{
			int key;
			int scancode;
			int action;
			int mods;
		};

		struct SP_CharStruct
		{
			unsigned int	charmods;
					 int	mods;
		};

		struct SP_PathStruct
		{
			int 			pathCount;
			const char** 	paths;
		};
		struct SP_ButtonStruct
		{
			int 			button;
			int 			action;
			int 			mods;
		};

		struct SP_Vec2dStruct {double x; double y;};
		struct SP_Vec2iStruct {int x; int y;};
		struct SP_VoidData{};

		SP_Detail(SP_SystemEvent e) :
			event 	{(long int)e},
			voidData{}
		{
		}

		SP_Detail(SP_SystemEvent e, int _1) :
			event  {(long int)e},
			intData{_1}
		{
		}

		SP_Detail(SP_SystemEvent e, int _1, int _2) :
			event  {(long int)e},
			v2iData{_1, _2}
		{
		}

		SP_Detail(SP_SystemEvent e, int _1, int _2, int _3) :
			event	   {(long int)e},
			buttonData {_1, _2, _3}
		{
		}

		SP_Detail(SP_SystemEvent e, int _1, int _2, int _3, int _4) :
			event		{(long int)e},
			keyboardData{_1, _2, _3, _4}
		{
		}

		SP_Detail(SP_SystemEvent e, unsigned _1) :
			event		{(long int)e},
			uintData	{ _1}
		{
		}

		SP_Detail(SP_SystemEvent e, unsigned int _1, int _2) :
			event		 {(long int)e},
			charmodsData {_1, _2}
		{
		}

		SP_Detail(SP_SystemEvent e, double _1, double _2) :
			event  {(long int)e},
			v2dData{_1, _2}
		{
		}

		SP_Detail(SP_SystemEvent e, int _1, const char* _2[]) :
			event	 {(long int)e},
			pathData {_1, _2}
		{
		}

		SP_Detail(SP_SystemEvent e, double _1) :
            event       {(long int)e},
            doubleData  {_1}
        {
        }

		long int		event;

		union
		{
			SP_Vec2iStruct		v2iData;
			SP_Vec2dStruct		v2dData;
			int					intData;
			unsigned int		uintData;
			SP_KeyStruct		keyboardData;
			SP_CharStruct		charmodsData;
			SP_PathStruct		pathData;
			SP_ButtonStruct		buttonData;
			SP_VoidData			voidData;
			double              doubleData;
		};
	};

	class UserEvent : public SP_Detail
	{
        public:
            UserEvent() : SP_Detail(SP_SystemEvent::UserEvent)
            {
            }

            virtual ~UserEvent()
            {
            }
            virtual long int name() = 0;
	};

	class CEvent : public UserEvent
	{
        public:
            CEvent(double x, double y) :
                UserEvent(),
                m_cursor_pos    {x, y}
            {
            }
            virtual long int name()
            {
                return 1;
            }
            vec2f       m_cursor_pos;
	};
}

#endif // SP_EVENT_H_INCLUDED
