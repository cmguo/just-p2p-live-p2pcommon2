
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_UTIL_TEST_CASE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_UTIL_TEST_CASE_H_

#include <ppl/util/log.h>
#include <ppl/util/macro.h>
#include <list>
#include <string>
#include <assert.h>

#ifdef _PPL_PLATFORM_MSWIN
#pragma warning(disable:4127)
#endif
/**
 * @file
 * @brief 一个简单的测试框架
 */



#define PPL_LOG_TYPE_TESTCASE 110

#define TESTCASE_EVENT(message) PPL_LOG_IMPL(PPL_LOG_TYPE_TESTCASE, _PPL_EVENT, message)




#if defined(_DEBUG) && !defined(_WIN32_WCE)

/// 表示启用测试框架(目前debug版自动运行)
#define _PPL_RUN_TEST

/// 兼容老的宏
#define _RUN_TEST

#ifdef _PPL_PLATFORM_MSWIN
#pragma message("------ enable automatical unit tests")
#endif

#endif


namespace ppl { namespace util {


#ifdef _PPL_RUN_TEST


/// 测试用例类
class test_case
{
public:
	test_case() { }
	virtual ~test_case() { }

	/// 初始化
	bool init()
	{
		return do_init();
	}

	/// 运行
	void run()
	{
		do_run();
	}

	void set_name(const std::string& name) { m_name = name; }
	const std::string& get_name() const { return m_name; }

protected:
	/// 初始化的实现
	virtual bool DoInit() { return true; }

	/// 运行的实现
	virtual void DoRun() { }

	/// 初始化的实现
	virtual bool do_init() { return DoInit(); }

	/// 运行的实现
	virtual void do_run() { DoRun(); }

private:
	std::string m_name;
};


typedef test_case TestCase;



/// 表示一组测试用例
class test_suite : public test_case
{
public:
	test_suite() { }
	~test_suite() { clear(); }

	/// 清除所有的测试用例
	void clear()
	{
		STL_FOR_EACH(test_case_collection, m_tests, iter)
		{
			test_case* tc = *iter;
			delete tc;
		}
		m_tests.clear();
	}

	/// 增加测试用例
	void add_test(test_case* tc)
	{
		m_tests.push_back(tc);
	}

	/// skeleton的实例
	static test_suite& instance()
	{
		static test_suite suite;
		return suite;
	}

protected:
	virtual void do_run()
	{
		TESTCASE_EVENT("Start running " << m_tests.size() << " test cases:");
		int i = 0;
		STL_FOR_EACH_CONST(test_case_collection, m_tests, iter)
		{
			test_case* tc = *iter;
			TESTCASE_EVENT("Start running test case " << i << ": " << tc->get_name());
			tc->run();
			i++;
		}
		TESTCASE_EVENT("All of " << m_tests.size() << " test cases passed.");
	}

private:
	/// 测试用例集合
	typedef std::list<test_case*> test_case_collection;
	/// 所包含的所有测试用例
	test_case_collection m_tests;
};



/// 测试用例的自动注册器
class auto_register_test_suite
{
public:
	explicit auto_register_test_suite(test_case* tc, const char* name)
	{
		tc->set_name(name);
		test_suite::instance().add_test(tc);
	}
};


/// 运行测试用例
inline void run_tests()
{
	test_suite::instance().run();
	test_suite::instance().clear();
}

#else

/// 运行测试用例(空实现)
inline void run_tests()
{
}

#endif

} }


/// 自动注册指定的测试用例
#define CPPUNIT_TEST_SUITE_REGISTRATION(testCase)	static ppl::util::auto_register_test_suite CPPUNIT_MAKE_UNIQUE_NAME(testCase)(new testCase, #testCase)

#ifdef _PPL_PLATFORM_MSWIN
#pragma warning(default:4127)
#endif

#endif