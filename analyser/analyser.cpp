#include "analyser.h"
#include "analyser.h"
#include "analyser.h"
#include "analyser.h"
#include "analyser.h"
#include "analyser.h"
#include "analyser.h"
#include "analyser.h"
#include "analyser.h"
#include "analyser.h"
#include "analyser.h"

#include <climits>
#include <sstream>
#include <string>

namespace miniplc0 {
	std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// <C0-program> ::= {<variable - declaration>} {<function - definition>}
	std::optional<CompilationError> Analyser::analyseProgram() {
		// 示例函数，示例如何调用子程序
		//<variable - declaration>
			auto err = analysevariable_declaration();
			if (err.has_value())
				return err;
		//<function - definition>
			err = function_definition();
			if (err.has_value())
				return err;
			return {};
	}

	//<variable - declaration> :: =
	//		[<const - qualifier>]<type - specifier> < init - declarator - list>';'
	//< init - declarator - list > :: =
	//		<init - declarator>{ ',' < init - declarator > }
	//<init - declarator> :: =
	//		<identifier>[<initializer>]
	//<initializer> :: =
	//		'=' < expression >
	std::optional<CompilationError> Analyser::analysevariable_declaration() {
		while (true) {
			auto next = nextToken();
			if (!next.has_value())
				return {};
			if (next.value().GetType() != TokenType::CONST && next.value().GetType() != TokenType::INT) {
				unreadToken();
				return {};
			}
			if (next.value().GetType() == TokenType::CONST) {
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::INT) {
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedInt);
				}
			}

			//< init - declarator - list >
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
			auto next2 = nextToken();
			if (!next2.has_value())
				return {};
			if (next2.value().GetType() == TokenType::EQUAL_SIGN) {
				// '<表达式>'
				auto err = analyseexpression();
				if (err.has_value())
					return err;
			}
			else {
				unreadToken();
			}
			//{ ',' < init - declarator > }
			while (true) {
				auto next = nextToken();
				if (!next.has_value())
					return {};
				if (next.value().GetType() != TokenType::COMMA) {
					unreadToken();
					return {};
				}
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
				auto next2 = nextToken();
				if (!next2.has_value())
					return {};
				if (next2.value().GetType() == TokenType::EQUAL_SIGN) {
					// '<表达式>'
					auto err = analyseexpression();
					if (err.has_value())
						return err;
				}
				else {
					unreadToken();
					return {};
				}
			}
			if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		}
	}

	/*<function - definition> :: =
		<type - specifier><identifier><parameter - clause><compound - statement>*/
	std::optional<CompilationError> Analyser::function_definition() {
		while (true) {
			auto next = nextToken();
			if (!next.has_value())
				return {};
			if (next.value().GetType() != TokenType::VOID && next.value().GetType() != TokenType::INT) {
				unreadToken();
				//是否要报错存疑
				return {};
			}

			next = nextToken();
			if (!next.has_value())
				return {};
			if (next.value().GetType() != TokenType::IDENTIFIER) {
				unreadToken();
				//是否要报错存疑
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
			}

			auto err = analyseparameter_clause();
			if (err.has_value())
				return err;

			// <语句序列>
			err = analysecompound_statement();
			if (err.has_value())
				return err;

			return {};
		}
	}

	/*<parameter - clause> :: =
		'('[<parameter - declaration - list>] ')'
	< parameter - declaration - list > :: =
		<parameter - declaration>{ ',' < parameter - declaration > }
	<parameter - declaration> :: =
		[<const - qualifier>]<type - specifier><identifier>*/
	std::optional<CompilationError> Analyser::analyseparameter_clause() {
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errparameter_clause);
		next = nextToken();
		if (!next.has_value())
				return {};
		if (next.value().GetType() == TokenType::CONST || next.value().GetType() == TokenType::INT) {
			if (next.value().GetType() == TokenType::CONST) {
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::INT) {
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedInt);
				}
			}
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errparameter_clause);
			}
			while (true) {
				auto next = nextToken();
				if (!next.has_value())
					return {};
				if (next.value().GetType() != TokenType::COMMA) {
					unreadToken();
					break;
				}
				next = nextToken();
				if (!next.has_value() || (next.value().GetType() != TokenType::CONST && next.value().GetType() != TokenType::INT)) {
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errparameter_clause);
				}
				if (next.value().GetType() == TokenType::CONST) {
					next = nextToken();
					if (!next.has_value() || next.value().GetType() != TokenType::INT) {
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedInt);
					}
				}
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER) {
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errparameter_clause);
				}
			}
		}
		next = nextToken();
		if(!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET){
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errparameter_clause);
		}
		return {};
	}

	/*<compound - statement> :: =
		'{' {<variable - declaration>} < statement - seq> '}'
	< statement - seq > :: =
		{ <statement> }*/
	std::optional<CompilationError> Analyser::analysecompound_statement(){
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACE)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errcompound_statement);
		auto err = analysevariable_declaration();
		if (err.has_value())
			return err;

		err = analysestatement_seq();
		if (err.has_value())
			return err;

		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACE)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errcompound_statement);
		return {};
	}

	std::optional<CompilationError> Analyser::analysestatement_seq() {
		while (true) {
			auto next = nextToken();
			if (!next.has_value())
				return {};
			unreadToken();
			if (next.value().GetType() != TokenType::LEFT_BRACE &&
				next.value().GetType() != TokenType::IF &&
				next.value().GetType() != TokenType::WHILE &&
				next.value().GetType() != TokenType::RETURN &&
				next.value().GetType() != TokenType::SCAN &&
				next.value().GetType() != TokenType::PRINT &&
				next.value().GetType() != TokenType::IDENTIFIER &&
				next.value().GetType() != TokenType::SEMICOLON) {
				return {};
			}
			auto err = analysestatement();
			if (err.has_value())
				return err;
		}
	}

	std::optional<CompilationError> Analyser::analysestatement() {
			auto next = nextToken();
			if (!next.has_value())
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);
			if (next.value().GetType() == TokenType::LEFT_BRACE) {
				auto err = analysestatement_seq();
				if (err.has_value())
					return err;
				next = nextToken();
				if(!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACE)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);
			}

			/*<condition - statement> :: =
				'if' '(' < condition > ')' < statement > ['else' < statement > ]*/
			else if (next.value().GetType() == TokenType::IF) {
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);
				auto err = analysecondition();
				if (err.has_value())
					return err;
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);

				err = analysestatement();
				if (err.has_value())
					return err;

				next = nextToken();
				if (!next.has_value() || next.value().GetType() == TokenType::ELSE) {
					auto err = analysestatement();
					if (err.has_value())
						return err;
				}
				else {
					unreadToken();
					return {};
				}
			}

			//<loop - statement> :: =
			//	'while' '(' < condition > ')' < statement >
			else if (next.value().GetType() == TokenType::WHILE) {
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);
				auto err = analysecondition();
				if (err.has_value())
					return err;
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);

				err = analysestatement();
				if (err.has_value())
					return err;

			}

			//<jump - statement> :: =
			//	<return-statement>
			//<return-statement> :: =
			//	'return'[<expression>] ';'
			else if (next.value().GetType() == TokenType::RETURN) {
				auto next = nextToken();
				if (!next.has_value())
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoEnd);
				if (next.value().GetType() != TokenType::SEMICOLON) {
					unreadToken();
					auto err = analyseexpression();
					if (err.has_value())
						return err;
					next = nextToken();
					if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
				}
				else {

				}
			}

			//<scan - statement> :: =
			//	'scan' '(' < identifier > ')' ';'
			else if (next.value().GetType() == TokenType::SCAN) {
				auto next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);
			}

			//<print - statement> :: =
			//	'print' '('[<printable - list>] ')' ';'
			//< printable - list > :: =
			//	<printable>{ ',' < printable > }
			//<printable> :: =
			//	<expression>
			else if (next.value().GetType() == TokenType::PRINT) {
				auto next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);
				next = nextToken();
				if (!next.has_value())
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoEnd);
				if (next.value().GetType() != TokenType::RIGHT_BRACKET) {
					unreadToken();
					auto err = analyseexpression();
					if (err.has_value())
						return err;
					while (true) {
						next = nextToken();
						if (!next.has_value())
							return {};
						if (next.value().GetType() != TokenType::COMMA) {
							unreadToken();
							break;
						}
						err = analyseexpression();
						if (err.has_value())
							return err;
					}
					next = nextToken();
					if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);
					next = nextToken();
					if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
				}
				else {
					next = nextToken();
					if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
				}
			}

			//<assignment - expression> :: =
			//	<identifier><assignment - operator><expression>
			else if (next.value().GetType() == TokenType::IDENTIFIER) {
				auto next = nextToken();
				if (!next.has_value())
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoEnd);
				//<assignment - expression> :: =
				//	<identifier><assignment - operator><expression>
				if (next.value().GetType() == TokenType::EQUAL_SIGN) {
					auto err = analyseexpression();
					if (err.has_value())
						return err;
					next = nextToken();
					if (!next.has_value())
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
					if (next.value().GetType() != TokenType::SEMICOLON) {
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
					}

				}

				else if (next.value().GetType() == TokenType::LEFT_BRACKET) {
					next = nextToken();
					if (!next.has_value())
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoEnd);
					if (next.value().GetType() != TokenType::RIGHT_BRACKET) {
						unreadToken();
						auto err = analyseexpression();
						if (err.has_value())
							return err;
						while (true) {
							next = nextToken();
							if (!next.has_value())
								return {};
							if (next.value().GetType() != TokenType::COMMA) {
								unreadToken();
								return {};
							}
							err = analyseexpression();
							if (err.has_value())
								return err;
						}
						next = nextToken();
						if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
							return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);
					}
					next = nextToken();
					if (!next.has_value())
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
					if (next.value().GetType() != TokenType::SEMICOLON) {
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
					}

				}
				else {
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);
				}
			}
			else if (next.value().GetType() == TokenType::SEMICOLON) {
				return {};
			}
			return {};
	}

	std::optional<CompilationError> Analyser::analyseexpression() {
		auto err = analysemultiplicative_expression();
		if (err.has_value())
			return err;
		while (true) {
			auto next = nextToken();
			if (!next.has_value())
				return {};
			auto type = next.value().GetType();
			if (type != TokenType::PLUS_SIGN && type != TokenType::MINUS_SIGN) {
				unreadToken();
				return {};
			}
			auto err = analysemultiplicative_expression();
			if (err.has_value())
				return err;
		}
		return {};
	}

	//<multiplicative - expression> :: =
	//	<unary - expression>{ <multiplicative - operator><unary - expression> }
	std::optional<CompilationError> Analyser::analysemultiplicative_expression() {
		auto err = analyseunary_expression();
		if (err.has_value())
			return err;
		while (true) {
			auto next = nextToken();
			if (!next.has_value())
				return {};
			auto type = next.value().GetType();
			if (type != TokenType::MULTIPLICATION_SIGN && type != TokenType::DIVISION_SIGN) {
				unreadToken();
				return {};
			}
			auto err = analyseunary_expression();
			if (err.has_value())
				return err;
		}
		return {};
	}

	//<unary - expression> :: =
	//	[<unary - operator>]<primary - expression>
	std::optional<CompilationError> Analyser::analyseunary_expression() {
		bool sign = true;
		auto next = nextToken();
		if (next.value().GetType() == MINUS_SIGN)
			sign = false;
		else if (next.value().GetType() == PLUS_SIGN)
			sign = true;
		else
			unreadToken();

		next = nextToken();
		if (!next.has_value())
			return {};
		if (next.value().GetType() == TokenType::LEFT_BRACKET) {
			auto err = analyseexpression();
			if (err.has_value())
				return err;
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errexpression);
		}
		else if (next.value().GetType() == TokenType::UNSIGNED_INTEGER) {
			return {};
		}
		else if (next.value().GetType() == TokenType::IDENTIFIER) {
			if (next.value().GetType() == TokenType::LEFT_BRACKET) {
				auto next = nextToken();
				if (!next.has_value())
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoEnd);
				if (!next.has_value())
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoEnd);
				if (next.value().GetType() != TokenType::RIGHT_BRACKET) {
					unreadToken();
					auto err = analyseexpression();
					if (err.has_value())
						return err;
					while (true) {
						next = nextToken();
						if (!next.has_value())
							return {};
						if (next.value().GetType() != TokenType::COMMA) {
							unreadToken();
							return {};
						}
						err = analyseexpression();
						if (err.has_value())
							return err;
					}
					next = nextToken();
					if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errstatement_seq);
				}
			}
			else {
				return {};
			}
		}
		else {
			return {};
		}
	}

	std::optional<CompilationError> Analyser::analysecondition() {
		auto err = analyseexpression();
		if (err.has_value())
			return err;
		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoEnd);

		if (next.value().GetType() == TokenType::LESS) {
			auto err = analyseexpression();
			if (err.has_value())
				return err;
		}
		else if (next.value().GetType() == TokenType::LE) {
			auto err = analyseexpression();
			if (err.has_value())
				return err;
		}
		else if (next.value().GetType() == TokenType::GREATER) {
			auto err = analyseexpression();
			if (err.has_value())
				return err;
		}
		else if (next.value().GetType() == TokenType::GE) {
			auto err = analyseexpression();
			if (err.has_value())
				return err;
		}
		else if (next.value().GetType() == TokenType::NE) {
			auto err = analyseexpression();
			if (err.has_value())
				return err;
		}
		else if (next.value().GetType() == TokenType::DE) {
			auto err = analyseexpression();
			if (err.has_value())
				return err;
		}
		else {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::Errcondition);
		}
		return {};
	}

	std::optional<Token> Analyser::nextToken() {
		if (_offset == _tokens.size())
			return {};
		// 考虑到 _tokens[0..._offset-1] 已经被分析过了
		// 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
		_current_pos = _tokens[_offset].GetEndPos();
		return _tokens[_offset++];
	}

	void Analyser::unreadToken() {
		if (_offset == 0)
			DieAndPrint("analyser unreads token from the begining.");
		_current_pos = _tokens[_offset - 1].GetEndPos();
		_offset--;
	}

	void Analyser::_add(const Token& tk, std::map<std::string, int32_t>& mp) {
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");
		mp[tk.GetValueString()] = _nextTokenIndex;
		_nextTokenIndex++;
	}

	void Analyser::addVariable(const Token& tk) {
		_add(tk, _vars);
	}

	void Analyser::addConstant(const Token& tk) {
		_add(tk, _consts);
	}

	void Analyser::addUninitializedVariable(const Token& tk) {
		_add(tk, _uninitialized_vars);
	}

	int32_t Analyser::getIndex(const std::string& s) {
		if (_uninitialized_vars.find(s) != _uninitialized_vars.end())
			return _uninitialized_vars[s];
		else if (_vars.find(s) != _vars.end())
			return _vars[s];
		else
			return _consts[s];
	}

	bool Analyser::isDeclared(const std::string& s) {
		return isConstant(s) || isUninitializedVariable(s) || isInitializedVariable(s);
	}

	bool Analyser::isUninitializedVariable(const std::string& s) {
		return _uninitialized_vars.find(s) != _uninitialized_vars.end();
	}
	bool Analyser::isInitializedVariable(const std::string& s) {
		return _vars.find(s) != _vars.end();
	}

	bool Analyser::isConstant(const std::string& s) {
		return _consts.find(s) != _consts.end();
	}
}