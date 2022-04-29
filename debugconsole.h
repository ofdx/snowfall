/*
	DebugConsole
	mperron (2022)

	Class which manages log messages and processed commands for the
	debug/development console.
*/
class DebugConsole {
	list<string> m_log;
	list<string> m_log_cmd;
	size_t m_logsize;

public:
	DebugConsole() :
	 	m_log(),
		m_logsize(100)
	{
		stringstream ss;
		
		ss << "-- " << GAME_NAME << " (" << GAME_VERSION << ") --" << endl;
		m_log.push_back(ss.str());
	}

	// Append a log message
	void log(string message){
		m_log.push_back(message);

		// Remove old messages over the scrollback cap.
		while(m_log.size() > m_logsize)
			m_log.pop_front();
	}
	void log_cmd(string cmd){
		if(cmd.empty()){
			log("");
			return;
		}

		log(string("   : ") + cmd);

		if(cmd != m_log_cmd.front())
			m_log_cmd.push_front(cmd);

		while(m_log_cmd.size() > m_logsize)
			m_log_cmd.pop_back();
	}

	void log_clear(){
		m_log.clear();
	}

	void cmd_at(unsigned int &index, string &buffer){
		if(index > m_log_cmd.size())
			index = m_log_cmd.size();

		if(index){
			int i = index;

			for(string cmd : m_log_cmd){
				if(!--i){
					buffer = cmd;
					return;
				}
			}
		}
	}

	// Retrieve the message log as a string.
	string log(){
		stringstream ss;

		// Empty space to pad the top of the log and improve scroll behavior.
		for(int i = 0; i < 13; i++)
			ss << endl;

		for(string s : m_log)
			ss << s << endl;

		return ss.str();
	}
};
