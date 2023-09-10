//
namespace sp
{
	class System
	{
		public:
									System(std::shared_ptr<Controller> ctrl);
									System() = delete;
						virtual    ~System() = default;
			
			virtual void			update(double dt) = 0;
		protected:
			std::shared_ptr<Controller>		m_controller;
	}
}
