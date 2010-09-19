
#include <gtest/gtest.h>

#include "cppdepinject/Repository.h"

using namespace std;
using namespace cppdepinject;

//=================================================================================================

struct A
{
	A() : a()  {}
	A(const string& a) : a(a)  {}

	string a;
};

struct B : public A
{
	B() : A()  {}
	B(const string& a) : A(a)  {}
};

struct C
{
	C(std::shared_ptr<A> a) : a(a)  {}

	std::shared_ptr<A> a;
};

auto IsA = [] (const std::string& name) -> bool { return name == "A"; };
auto StartsA = [] (const std::string& name) -> bool { return name.substr(0,1) == "A"; };


//=================================================================================================

TEST(CppInjectTest, resolveNoArgs_EmptyRepository_ThrowsException) {
	Repository r;
	ASSERT_THROW( r.resolve<A>(), RepositoryException );
}

TEST(CppInjectTest, resolveNoArgs_HasUnnamedObject_ReturnsObject) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );

	ASSERT_TRUE(r.resolve<A>());
}

TEST(CppInjectTest, resolveNoArgs_HasNamedObject_ReturnsObject) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "A" );

	ASSERT_TRUE(r.resolve<A>());
}

TEST(CppInjectTest, resolveNoArgs_HasTwoObjects_ThrowsException) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );
	r.registerInstance<A>( SimpleCreator<A>(), "A" );

	ASSERT_THROW( r.resolve<A>(), RepositoryException );
}

//=================================================================================================

TEST(CppInjectTest, resolveByName_EmptyRepository_ThrowsException) {
	Repository r;
	ASSERT_THROW( r.resolveByName<A>("A"), RepositoryException );
}

TEST(CppInjectTest, resolveByName_HasUnnamedObject_ReturnsObject) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );

	ASSERT_THROW( r.resolveByName<A>("A"), RepositoryException );
}

TEST(CppInjectTest, resolveByName_HasCorrectNamedObject_ReturnsObject) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "A" );

	ASSERT_TRUE(r.resolveByName<A>("A"));
}

TEST(CppInjectTest, resolveByName_HasOtherObject_ThrowsException) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "!A" );

	ASSERT_THROW( r.resolveByName<A>("A"), RepositoryException );
}

TEST(CppInjectTest, resolveByName_HasTwoObjects_ReturnsCorrectObject) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );
	r.registerInstanceObject<A>( shared_ptr<A>(new A("a")), "A" );
	r.registerInstanceObject<A>( shared_ptr<A>(new A("b")), "B" );

	shared_ptr<A> a( r.resolveByName<A>("A") );
	ASSERT_TRUE(a);
	ASSERT_EQ("a", a->a);
}

//=================================================================================================

TEST(CppInjectTest, resolveFilter_EmptyRepository_ThrowsException) {
	Repository r;

	ASSERT_THROW( r.resolve<A>( IsA ), RepositoryException );
}

TEST(CppInjectTest, resolveFilter_HasMatchingObject_ReturnsObject) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "A" );

	ASSERT_TRUE( r.resolve<A>( IsA ) );
}

TEST(CppInjectTest, resolveFilter_HasNonMatchingObject_ThrowsException) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "!A" );

	ASSERT_THROW( r.resolve<A>( IsA ), RepositoryException );
}

TEST(CppInjectTest, resolveFilter_HasTwoObjects_ReturnsCorrectObject) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );
	r.registerInstanceObject<A>( shared_ptr<A>(new A("a")), "A" );
	r.registerInstanceObject<A>( shared_ptr<A>(new A("b")), "B" );

	// lambda to filter on registered name
	shared_ptr<A> a( r.resolve<A>( IsA ) );
	ASSERT_TRUE(a);
	ASSERT_EQ("a", a->a);
}

TEST(CppInjectTest, resolveFilter_HasTwoMatchingObjects_ThrowsException) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "Aa" );
	r.registerInstance<A>( SimpleCreator<A>(), "Ab" );

	ASSERT_THROW( r.resolve<A>( StartsA ), RepositoryException );
}

//=================================================================================================

TEST(CppInjectTest, resolveAllFilter_EmptyRepository_ReturnsEmpty) {
	Repository r;

	ASSERT_TRUE( r.resolveAll<A>( IsA ).empty() );
}

TEST(CppInjectTest, resolveAllFilter_HasMatchingObject_ReturnsObject) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "A" );

	auto found = r.resolveAll<A>( IsA );
	ASSERT_EQ( 1, found.size() );
}

TEST(CppInjectTest, resolveAllFilter_HasNonMatchingObject_ReturnsEmpty) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "!A" );

	ASSERT_TRUE( r.resolveAll<A>( IsA ).empty() );
}

TEST(CppInjectTest, resolveAllFilter_HasTwoObjects_ReturnsCorrectObject) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );
	r.registerInstanceObject<A>( shared_ptr<A>(new A("a")), "A" );
	r.registerInstanceObject<A>( shared_ptr<A>(new A("b")), "B" );

	auto found = r.resolveAll<A>( IsA );
	ASSERT_EQ( 1, found.size() );
	ASSERT_EQ("a", found.begin()->second->a);
}

TEST(CppInjectTest, resolveAllFilter_HasTwoMatchingObjects_ReturnsBoth) {
	Repository r;
	r.registerInstanceObject<A>( shared_ptr<A>(new A("aa")), "Aa" );
	r.registerInstanceObject<A>( shared_ptr<A>(new A("ab")), "Ab" );

	auto found = r.resolveAll<A>( StartsA );
	ASSERT_EQ( 2, found.size() );
	ASSERT_EQ("aa", found.begin()->second->a);
	ASSERT_EQ("ab", (++found.begin())->second->a);
}

//=================================================================================================

TEST(CppInjectTest, resolveAllDefault_EmptyRepository_ReturnsEmpty) {
	Repository r;

	ASSERT_TRUE( r.resolveAll<A>().empty() );
}

TEST(CppInjectTest, resolveAllDefault_HasOneObject_ReturnsObject) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );

	auto found = r.resolveAll<A>();
	ASSERT_EQ( 1, found.size() );
}

TEST(CppInjectTest, resolveAllDefault_HasTwoObjects_ReturnsBoth) {
	Repository r;
	r.registerInstanceObject<A>( shared_ptr<A>(new A("aa")), "Aa" );
	r.registerInstanceObject<A>( shared_ptr<A>(new A("ab")), "Ab" );

	auto found = r.resolveAll<A>();
	ASSERT_EQ( 2, found.size() );
	ASSERT_EQ("aa", found.begin()->second->a);
	ASSERT_EQ("ab", (++found.begin())->second->a);
}

//=================================================================================================

TEST(CppInjectTest, resolve_ObjectNeedingOtherObjectRegisteredFirst_ReturnsCorrectlyConstructedObject) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );
	r.registerInstance<C>( SimpleCreator_OneResolved<C, A>() );

	auto c = r.resolve<C>();
	ASSERT_TRUE( c );
	ASSERT_TRUE( c->a );
}

TEST(CppInjectTest, resolve_ObjectNeedingOtherObjectRegisteredAfter_ReturnsCorrectlyConstructedObject) {
	Repository r;
	r.registerInstance<C>( SimpleCreator_OneResolved<C, A>() );
	r.registerInstance<A>( SimpleCreator<A>() );

	auto c = r.resolve<C>();
	ASSERT_TRUE( c );
	ASSERT_TRUE( c->a );
}

//=================================================================================================

TEST(CppInjectTest, resolve_AfterRegisterInstanceObject_ReturnsGivenObjectEachTime) {
	Repository r;
	std::shared_ptr<A> a(new A);
	r.registerInstanceObject<A>(a);

	ASSERT_TRUE( a == r.resolve<A>() );
	ASSERT_TRUE( a == r.resolve<A>() );
}

TEST(CppInjectTest, resolve_AfterRegisterInstance_ReturnsSameObjectEachTime) {
	Repository r;
	r.registerInstance<A>([] (const Repository&) { return shared_ptr<A>(new B); });

	ASSERT_TRUE( r.resolve<A>() == r.resolve<A>() );
}

TEST(CppInjectTest, resolve_AfterRegisterType_ReturnsDifferentObjectEachTime) {
	Repository r;
	r.registerType<A>([] (const Repository&) { return shared_ptr<A>(new B); });

	ASSERT_TRUE( r.resolve<A>() != r.resolve<A>() );
}

//=================================================================================================
