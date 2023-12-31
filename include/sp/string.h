#ifndef STRING_XDD__INCLUDE
#define STRING_XDD__INCLUDE
#include <sp/sp.h>
#include <sp/utf.h>
#include <string>
#include <vector>
#include <cstring>
#include <locale>
#include <iomanip>
#include <ostream>
#include <sstream>

namespace sp
{
    bool isWhitespace(char character);
    bool isWhitespace(char32_t character);

    class String
    {
    private:
        std::u32string m_string;

#if SP_COMPILE_VERSION >= 17
        template <typename StringViewType>
        using IsStringViewType = std::enable_if_t<
            std::is_same_v<StringViewType, std::string_view>
#if defined(__cpp_lib_char8_t) && (__cpp_lib_char8_t >= 201811L)
            || std::is_same_v<StringViewType, std::u8string_view>
#endif
            || std::is_same_v<StringViewType, std::wstring_view>
            || std::is_same_v<StringViewType, std::u16string_view>
            || std::is_same_v<StringViewType, std::u32string_view>,
            void>;
#endif

    public:

        static const decltype(std::u32string::npos) npos;

        using iterator = std::u32string::iterator;
        using const_iterator = std::u32string::const_iterator;
        using reverse_iterator = std::u32string::reverse_iterator;
        using const_reverse_iterator = std::u32string::const_reverse_iterator;

        using value_type = char32_t;
        using reference = char32_t&;
        using const_reference = const char32_t&;


    public:
        bool attemptToInt(int& result) const;
        bool attemptToUInt(unsigned int& result) const;
        bool attemptToFloat(float& result) const;
        int toInt(int defaultValue = 0) const;
        unsigned int toUInt(unsigned int defaultValue = 0) const;
        float toFloat(float defaultValue = 0) const;
        String trim() const;
        String toLower() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Converts the ASCII characters in the string to uppercase
        ///
        /// @return Uppercase string
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        String toUpper() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Compares this string to another and checks if they are equal if ASCII letters would have been lowercase
        ///
        /// @param other  The other string to compare this one with
        ///
        /// @return Are the strings equal except for the case of ASCII letters?
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        bool equalIgnoreCase(const String& other) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Checks whether the first part of the string matches the given substring
        ///
        /// @param substring  Characters to compare against the first part of the string
        ///
        /// @return Does the first part of the string match the given substring?
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        bool startsWith(const String& substring) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Checks whether the first part of the string matches the given substring, case-insensitive
        ///
        /// @param substring  Characters to compare against the first part of the string
        ///
        /// @return Does the first part of the string match the given substring if we ignore the case?
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        bool startsWithIgnoreCase(const String& substring) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Checks whether the last part of the string matches the given substring
        ///
        /// @param substring  Characters to compare against the final part of the string
        ///
        /// @return Does the back of the string match the given substring?
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        bool endsWith(const String& substring) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Checks whether the last part of the string matches the given substring, case-insensitive
        ///
        /// @param substring  Characters to compare against the final part of the string
        ///
        /// @return Does the back of the string match the given substring if we ignore the case?
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        bool endsWithIgnoreCase(const String& substring) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Replaces all occurrences of a substring with a replacement string
        ///
        /// @param searchFor   The value being searched for
        /// @param replaceWith The value that replaces found searchFor values
        ///
        /// @return Reference to this object.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        String& replace(const String& searchFor, const String& replaceWith);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Splits the string into multiple substrings given the delimiter that separates them
        ///
        /// @param delimiter  Character that separates the substrings
        /// @param trim       Should whitespace be removed at the start and end of each part?
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        std::vector<String> split(char32_t delimiter, bool trim = false) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Joins multiple string segments into a single string
        ///
        /// @param segments   Substrings that need to concatenated behind each other (with optional separators inbetween)
        /// @param separator  Character that separates the substrings
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        static String join(const std::vector<String>& segments, const String& separator);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Construct the string from a number
        ///
        /// @param value  Number to convert to string
        ///
        /// @return String representing given number
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        template <typename T>
        static String fromNumber(T value)
        {
            std::ostringstream oss;
            oss.imbue(std::locale::classic());
            oss << value;
            return String(oss.str());
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Construct the string from a floating point number, keeping only a certain amount of decimals behind the comma
        ///
        /// @param value     Number to convert to string
        /// @param decimals  Digits to keep behind the comma
        ///
        /// @return String representing given number, rounded to the given decimals
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        template <typename T>
        static String fromNumberRounded(T value, int decimals)
        {
            std::ostringstream oss;
            oss.imbue(std::locale::classic());
            oss << std::fixed << std::setprecision(decimals);
            oss << value;
            return String(oss.str());
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    public:

        String() = default;

        String(const std::string& str);
        String(const std::wstring& str);
        String(const std::u16string& str);
        String(const std::u32string& str);

        String(std::u32string&& str) :
            m_string{std::move(str)}
        {
        }

        String(char ansiChar);
        String(wchar_t wideChar);
        String(char16_t utfChar);
        String(char32_t utfChar);

        String(const char* str);
        String(const wchar_t* str);
        String(const char16_t* str);
        String(const char32_t* str) :
            m_string{str}
        {
        }

        // Constructor to initialize the string from a number (integer or float)
        template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
        explicit String(T number) :
            String{fromNumber(number)}
        {
        }

        String(std::size_t count, char ch);
        String(std::size_t count, wchar_t ch);
        String(std::size_t count, char16_t ch);
        String(std::size_t count, char32_t ch);

        String(const std::string& str, std::size_t pos);
        String(const std::wstring& str, std::size_t pos);
        String(const std::u16string& str, std::size_t pos);
        String(const std::u32string& str, std::size_t pos);

        String(const std::string& str, std::size_t pos, std::size_t count);
        String(const std::wstring& str, std::size_t pos, std::size_t count);
        String(const std::u16string& str, std::size_t pos, std::size_t count);
        String(const std::u32string& str, std::size_t pos, std::size_t count);

        String(const char* str, std::size_t count);
        String(const wchar_t* str, std::size_t count);
        String(const char16_t* str, std::size_t count);
        String(const char32_t* str, std::size_t count);

        explicit String(std::initializer_list<char> chars);
        explicit String(std::initializer_list<wchar_t> chars);
        explicit String(std::initializer_list<char16_t> chars);
        explicit String(std::initializer_list<char32_t> chars);

        // Constructors using iterators have to be explicit to prevent {"1", "2"} to be ambiguous between String and std::vector<String>.
        // The reason these constructors were considered a candicate to clang is due to a private constructor in the iterator class.
        explicit String(std::string::const_iterator first, std::string::const_iterator last);
        explicit String(std::wstring::const_iterator first, std::wstring::const_iterator last);
        explicit String(std::u16string::const_iterator first, std::u16string::const_iterator last);
        explicit String(std::u32string::const_iterator first, std::u32string::const_iterator last);

#if SP_COMPILE_VERSION >= 17
        template <typename StringViewType, typename = IsStringViewType<StringViewType>>
        explicit String(const StringViewType& stringView) :
            String(stringView.data(), stringView.size())
        {
        }

        template <typename StringViewType, typename = IsStringViewType<StringViewType>>
        explicit String(const StringViewType& stringView, std::size_t pos, std::size_t count) :
            String(stringView.data(), pos, count)
        {
        }
#endif
        explicit operator std::string() const;
        explicit operator std::wstring() const;
        explicit operator std::u16string() const;
        explicit operator const std::u32string&() const
        {
            return m_string;
        }

#if SP_COMPILE_VERSION >= 17
        operator std::u32string_view() const noexcept
        {
            return m_string;
        }
#endif

        std::string toStdString() const;
        std::wstring toWideString() const;
        std::u16string toUtf16() const;
        const std::u32string& toUtf32() const
        {
            return m_string;
        }

        String& assign(std::size_t count, char ch);
        String& assign(std::size_t count, wchar_t ch);
        String& assign(std::size_t count, char16_t ch);
        String& assign(std::size_t count, char32_t ch);

        String& assign(const std::string& str);
        String& assign(const std::wstring& str);
        String& assign(const std::u16string& str);
        String& assign(const std::u32string& str);
        String& assign(const String& str);

        String& assign(const std::string& str, std::size_t pos, std::size_t count = npos);
        String& assign(const std::wstring& str, std::size_t pos, std::size_t count = npos);
        String& assign(const std::u16string& str, std::size_t pos, std::size_t count = npos);
        String& assign(const std::u32string& str, std::size_t pos, std::size_t count = npos);
        String& assign(const String& str, std::size_t pos, std::size_t count = npos);

        String& assign(std::string&& str);
        String& assign(std::wstring&& str);
        String& assign(std::u16string&& str);
        String& assign(std::u32string&& str);
        String& assign(String&& str);

        String& assign(const char* str, std::size_t count);
        String& assign(const wchar_t* str, std::size_t count);
        String& assign(const char16_t* str, std::size_t count);
        String& assign(const char32_t* str, std::size_t count);

        String& assign(const char* str);
        String& assign(const wchar_t* str);
        String& assign(const char16_t* str);
        String& assign(const char32_t* str);

        String& assign(std::initializer_list<char> chars);
        String& assign(std::initializer_list<wchar_t> chars);
        String& assign(std::initializer_list<char16_t> chars);
        String& assign(std::initializer_list<char32_t> chars);

        String& assign(std::string::const_iterator first, std::string::const_iterator last);
        String& assign(std::wstring::const_iterator first, std::wstring::const_iterator last);
        String& assign(std::u16string::const_iterator first, std::u16string::const_iterator last);
        String& assign(std::u32string::const_iterator first, std::u32string::const_iterator last);

        reference       at(std::size_t pos);
        const_reference at(std::size_t pos) const;

        const_reference operator [](std::size_t index) const;
        reference operator [](std::size_t index);

        reference       front();
        const_reference front() const;

        reference       back();
        const_reference back() const;

        const char32_t* data() const noexcept
        {
            return m_string.data();
        }

#if __cplusplus >= 201703L
        char32_t* data() noexcept
        {
            return m_string.data();
        }
#endif
        const char32_t* c_str() const noexcept
        {
            return m_string.c_str();
        }

        iterator begin() noexcept;
        const_iterator begin() const noexcept;
        const_iterator cbegin() const noexcept;

        iterator end() noexcept;
        const_iterator end() const noexcept;
        const_iterator cend() const noexcept;

        reverse_iterator rbegin() noexcept;
        const_reverse_iterator rbegin() const noexcept;
        const_reverse_iterator crbegin() const noexcept;

        reverse_iterator rend() noexcept;
        const_reverse_iterator rend() const noexcept;
        const_reverse_iterator crend() const noexcept;

        bool empty() const noexcept
        {
            return m_string.empty();
        }

        std::size_t size() const noexcept
        {
            return m_string.size();
        }

        std::size_t length() const noexcept
        {
            return m_string.length();
        }

        std::size_t max_size() const noexcept;

        void reserve(std::size_t newCap);
        std::size_t capacity() const noexcept;
        void shrink_to_fit();

        void clear() noexcept;

        String& insert(std::size_t index, std::size_t count, char ch);
        String& insert(std::size_t index, std::size_t count, wchar_t ch);
        String& insert(std::size_t index, std::size_t count, char16_t ch);
        String& insert(std::size_t index, std::size_t count, char32_t ch);

        String& insert(std::size_t index, const std::string& str);
        String& insert(std::size_t index, const std::wstring& str);
        String& insert(std::size_t index, const std::u16string& str);
        String& insert(std::size_t index, const std::u32string& str);
        String& insert(std::size_t index, const String& str);

        String& insert(std::size_t index, const std::string& str, std::size_t pos, std::size_t count = npos);
        String& insert(std::size_t index, const std::wstring& str, std::size_t pos, std::size_t count = npos);
        String& insert(std::size_t index, const std::u16string& str, std::size_t pos, std::size_t count = npos);
        String& insert(std::size_t index, const std::u32string& str, std::size_t pos, std::size_t count = npos);
        String& insert(std::size_t index, const String& str, std::size_t pos, std::size_t count = npos);

        String& insert(std::size_t index, const char* str, std::size_t count);
        String& insert(std::size_t index, const wchar_t* str, std::size_t count);
        String& insert(std::size_t index, const char16_t* str, std::size_t count);
        String& insert(std::size_t index, const char32_t* str, std::size_t count);

        String& insert(std::size_t index, const char* str);
        String& insert(std::size_t index, const wchar_t* str);
        String& insert(std::size_t index, const char16_t* str);
        String& insert(std::size_t index, const char32_t* str);

        iterator insert(const_iterator pos, char ch);
        iterator insert(const_iterator pos, wchar_t ch);
        iterator insert(const_iterator pos, char16_t ch);
        iterator insert(const_iterator pos, char32_t ch);

        iterator insert(const_iterator pos, std::size_t count, char ch);
        iterator insert(const_iterator pos, std::size_t count, wchar_t ch);
        iterator insert(const_iterator pos, std::size_t count, char16_t ch);
        iterator insert(const_iterator pos, std::size_t count, char32_t ch);

        iterator insert(const_iterator pos, std::initializer_list<char> chars);
        iterator insert(const_iterator pos, std::initializer_list<wchar_t> chars);
        iterator insert(const_iterator pos, std::initializer_list<char16_t> chars);
        iterator insert(const_iterator pos, std::initializer_list<char32_t> chars);

        iterator insert(const_iterator pos, std::string::const_iterator first, std::string::const_iterator last);
        iterator insert(const_iterator pos, std::wstring::const_iterator first, std::wstring::const_iterator last);
        iterator insert(const_iterator pos, std::u16string::const_iterator first, std::u16string::const_iterator last);
        iterator insert(const_iterator pos, std::u32string::const_iterator first, std::u32string::const_iterator last);

        String& erase(std::size_t index = 0, std::size_t count = npos);

        iterator erase(const_iterator position);
        iterator erase(const_iterator first, const_iterator last);

        void push_back(char ch);
        void push_back(wchar_t ch);
        void push_back(char16_t ch);
        void push_back(char32_t ch);

        void pop_back();

        String& append(std::size_t count, char ch);
        String& append(std::size_t count, wchar_t ch);
        String& append(std::size_t count, char16_t ch);
        String& append(std::size_t count, char32_t ch);

        String& append(const std::string& str);
        String& append(const std::wstring& str);
        String& append(const std::u16string& str);
        String& append(const std::u32string& str);
        String& append(const String& str);

        String& append(const std::string& str, std::size_t pos, std::size_t count = npos);
        String& append(const std::wstring& str, std::size_t pos, std::size_t count = npos);
        String& append(const std::u16string& str, std::size_t pos, std::size_t count = npos);
        String& append(const std::u32string& str, std::size_t pos, std::size_t count = npos);
        String& append(const String& str, std::size_t pos, std::size_t count = npos);

        String& append(const char* str, std::size_t count);
        String& append(const wchar_t* str, std::size_t count);
        String& append(const char16_t* str, std::size_t count);
        String& append(const char32_t* str, std::size_t count);

        String& append(const char* str);
        String& append(const wchar_t* str);
        String& append(const char16_t* str);
        String& append(const char32_t* str);

        String& append(std::string::const_iterator first, std::string::const_iterator last);
        String& append(std::wstring::const_iterator first, std::wstring::const_iterator last);
        String& append(std::u16string::const_iterator first, std::u16string::const_iterator last);
        String& append(std::u32string::const_iterator first, std::u32string::const_iterator last);

        String& append(std::initializer_list<char> chars);
        String& append(std::initializer_list<wchar_t> chars);
        String& append(std::initializer_list<char16_t> chars);
        String& append(std::initializer_list<char32_t> chars);

        String& operator+=(const String& str);

        int compare(const std::string& str) const noexcept;
        int compare(const std::wstring& str) const noexcept;
        int compare(const std::u16string& str) const noexcept;
        int compare(const std::u32string& str) const noexcept;
        int compare(const String& str) const noexcept;

        int compare(std::size_t pos1, std::size_t count1, const std::string& str) const;
        int compare(std::size_t pos1, std::size_t count1, const std::wstring& str) const;
        int compare(std::size_t pos1, std::size_t count1, const std::u16string& str) const;
        int compare(std::size_t pos1, std::size_t count1, const std::u32string& str) const;
        int compare(std::size_t pos1, std::size_t count1, const String& str) const;

        int compare(std::size_t pos1, std::size_t count1, const std::string& str, std::size_t pos2, std::size_t count2 = npos) const;
        int compare(std::size_t pos1, std::size_t count1, const std::wstring& str, std::size_t pos2, std::size_t count2 = npos) const;
        int compare(std::size_t pos1, std::size_t count1, const std::u16string& str, std::size_t pos2, std::size_t count2 = npos) const;
        int compare(std::size_t pos1, std::size_t count1, const std::u32string& str, std::size_t pos2, std::size_t count2 = npos) const;
        int compare(std::size_t pos1, std::size_t count1, const String& str, std::size_t pos2, std::size_t count2 = npos) const;

        int compare(const char* s) const;
        int compare(const wchar_t* s) const;
        int compare(const char16_t* s) const;
        int compare(const char32_t* s) const;

        int compare(std::size_t pos1, std::size_t count1, const char* s) const;
        int compare(std::size_t pos1, std::size_t count1, const wchar_t* s) const;
        int compare(std::size_t pos1, std::size_t count1, const char16_t* s) const;
        int compare(std::size_t pos1, std::size_t count1, const char32_t* s) const;

        int compare(std::size_t pos1, std::size_t count1, const char* s, std::size_t count2) const;
        int compare(std::size_t pos1, std::size_t count1, const wchar_t* s, std::size_t count2) const;
        int compare(std::size_t pos1, std::size_t count1, const char16_t* s, std::size_t count2) const;
        int compare(std::size_t pos1, std::size_t count1, const char32_t* s, std::size_t count2) const;

        String& replace(std::size_t pos, std::size_t count, const std::string& str);
        String& replace(std::size_t pos, std::size_t count, const std::wstring& str);
        String& replace(std::size_t pos, std::size_t count, const std::u16string& str);
        String& replace(std::size_t pos, std::size_t count, const std::u32string& str);
        String& replace(std::size_t pos, std::size_t count, const String& str);

        String& replace(const_iterator first, const_iterator last, const std::string& str);
        String& replace(const_iterator first, const_iterator last, const std::wstring& str);
        String& replace(const_iterator first, const_iterator last, const std::u16string& str);
        String& replace(const_iterator first, const_iterator last, const std::u32string& str);
        String& replace(const_iterator first, const_iterator last, const String& str);

        String& replace(std::size_t pos, std::size_t count, const std::string& str, std::size_t pos2, std::size_t count2 = npos);
        String& replace(std::size_t pos, std::size_t count, const std::wstring& str, std::size_t pos2, std::size_t count2 = npos);
        String& replace(std::size_t pos, std::size_t count, const std::u16string& str, std::size_t pos2, std::size_t count2 = npos);
        String& replace(std::size_t pos, std::size_t count, const std::u32string& str, std::size_t pos2, std::size_t count2 = npos);
        String& replace(std::size_t pos, std::size_t count, const String& str, std::size_t pos2, std::size_t count2 = npos);

        String& replace(const_iterator first, const_iterator last, std::string::const_iterator first2, std::string::const_iterator last2);
        String& replace(const_iterator first, const_iterator last, std::wstring::const_iterator first2, std::wstring::const_iterator last2);
        String& replace(const_iterator first, const_iterator last, std::u16string::const_iterator first2, std::u16string::const_iterator last2);
        String& replace(const_iterator first, const_iterator last, std::u32string::const_iterator first2, std::u32string::const_iterator last2);

        String& replace(std::size_t pos, std::size_t count, const char* cstr, std::size_t count2);
        String& replace(std::size_t pos, std::size_t count, const wchar_t* cstr, std::size_t count2);
        String& replace(std::size_t pos, std::size_t count, const char16_t* cstr, std::size_t count2);
        String& replace(std::size_t pos, std::size_t count, const char32_t* cstr, std::size_t count2);

        String& replace(const_iterator first, const_iterator last, const char* cstr, std::size_t count2);
        String& replace(const_iterator first, const_iterator last, const wchar_t* cstr, std::size_t count2);
        String& replace(const_iterator first, const_iterator last, const char16_t* cstr, std::size_t count2);
        String& replace(const_iterator first, const_iterator last, const char32_t* cstr, std::size_t count2);

        String& replace(std::size_t pos, std::size_t count, const char* cstr);
        String& replace(std::size_t pos, std::size_t count, const wchar_t* cstr);
        String& replace(std::size_t pos, std::size_t count, const char16_t* cstr);
        String& replace(std::size_t pos, std::size_t count, const char32_t* cstr);

        String& replace(const_iterator first, const_iterator last, const char* cstr);
        String& replace(const_iterator first, const_iterator last, const wchar_t* cstr);
        String& replace(const_iterator first, const_iterator last, const char16_t* cstr);
        String& replace(const_iterator first, const_iterator last, const char32_t* cstr);

        String& replace(std::size_t pos, std::size_t count, std::size_t count2, char ch);
        String& replace(std::size_t pos, std::size_t count, std::size_t count2, wchar_t ch);
        String& replace(std::size_t pos, std::size_t count, std::size_t count2, char16_t ch);
        String& replace(std::size_t pos, std::size_t count, std::size_t count2, char32_t ch);

        String& replace(const_iterator first, const_iterator last, std::size_t count2, char ch);
        String& replace(const_iterator first, const_iterator last, std::size_t count2, wchar_t ch);
        String& replace(const_iterator first, const_iterator last, std::size_t count2, char16_t ch);
        String& replace(const_iterator first, const_iterator last, std::size_t count2, char32_t ch);

        String& replace(const_iterator first, const_iterator last, std::initializer_list<char> chars);
        String& replace(const_iterator first, const_iterator last, std::initializer_list<wchar_t> chars);
        String& replace(const_iterator first, const_iterator last, std::initializer_list<char16_t> chars);
        String& replace(const_iterator first, const_iterator last, std::initializer_list<char32_t> chars);

        String substr(std::size_t pos = 0, std::size_t count = npos) const;

        std::size_t copy(char32_t* dest, std::size_t count, std::size_t pos = 0) const;

        void resize(std::size_t count);
        void resize(std::size_t count, char ch);
        void resize(std::size_t count, wchar_t ch);
        void resize(std::size_t count, char16_t ch);
        void resize(std::size_t count, char32_t ch);

        void swap(String& other);

#if SP_COMPILE_VERSION >= 17
        bool contains(std::u32string_view sv) const noexcept;
#endif

        bool contains(char c) const noexcept;
        bool contains(wchar_t c) const noexcept;
        bool contains(char16_t c) const noexcept;
        bool contains(char32_t c) const noexcept;

        bool contains(const char* s) const;
        bool contains(const wchar_t* s) const;
        bool contains(const char16_t* s) const;
        bool contains(const char32_t* s) const;

        std::size_t find(const std::string& str, std::size_t pos = 0) const noexcept;
        std::size_t find(const std::wstring& str, std::size_t pos = 0) const noexcept;
        std::size_t find(const std::u16string& str, std::size_t pos = 0) const noexcept;
        std::size_t find(const std::u32string& str, std::size_t pos = 0) const noexcept;
        std::size_t find(const String& str, std::size_t pos = 0) const noexcept;

        std::size_t find(const char* s, std::size_t pos, std::size_t count) const;
        std::size_t find(const wchar_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find(const char16_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find(const char32_t* s, std::size_t pos, std::size_t count) const;

        std::size_t find(const char* s, std::size_t pos = 0) const;
        std::size_t find(const wchar_t* s, std::size_t pos = 0) const;
        std::size_t find(const char16_t* s, std::size_t pos = 0) const;
        std::size_t find(const char32_t* s, std::size_t pos = 0) const;

        std::size_t find(char ch, std::size_t pos = 0) const noexcept;
        std::size_t find(wchar_t ch, std::size_t pos = 0) const noexcept;
        std::size_t find(char16_t ch, std::size_t pos = 0) const noexcept;
        std::size_t find(char32_t ch, std::size_t pos = 0) const noexcept;

#if SP_COMPILE_VERSION >= 17
        std::size_t find(std::u32string_view sv, std::size_t pos = 0) const noexcept;
#endif

        std::size_t find_first_of(const std::string& str, std::size_t pos = 0) const noexcept;
        std::size_t find_first_of(const std::wstring& str, std::size_t pos = 0) const noexcept;
        std::size_t find_first_of(const std::u16string& str, std::size_t pos = 0) const noexcept;
        std::size_t find_first_of(const std::u32string& str, std::size_t pos = 0) const noexcept;
        std::size_t find_first_of(const String& str, std::size_t pos = 0) const noexcept;

        std::size_t find_first_of(const char* s, std::size_t pos, std::size_t count) const;
        std::size_t find_first_of(const wchar_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find_first_of(const char16_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find_first_of(const char32_t* s, std::size_t pos, std::size_t count) const;

        std::size_t find_first_of(const char* s, std::size_t pos = 0) const;
        std::size_t find_first_of(const wchar_t* s, std::size_t pos = 0) const;
        std::size_t find_first_of(const char16_t* s, std::size_t pos = 0) const;
        std::size_t find_first_of(const char32_t* s, std::size_t pos = 0) const;

        std::size_t find_first_of(char ch, std::size_t pos = 0) const noexcept;
        std::size_t find_first_of(wchar_t ch, std::size_t pos = 0) const noexcept;
        std::size_t find_first_of(char16_t ch, std::size_t pos = 0) const noexcept;
        std::size_t find_first_of(char32_t ch, std::size_t pos = 0) const noexcept;

        std::size_t find_first_not_of(const std::string& str, std::size_t pos = 0) const noexcept;
        std::size_t find_first_not_of(const std::wstring& str, std::size_t pos = 0) const noexcept;
        std::size_t find_first_not_of(const std::u16string& str, std::size_t pos = 0) const noexcept;
        std::size_t find_first_not_of(const std::u32string& str, std::size_t pos = 0) const noexcept;
        std::size_t find_first_not_of(const String& str, std::size_t pos = 0) const noexcept;

        std::size_t find_first_not_of(const char* s, std::size_t pos, std::size_t count) const;
        std::size_t find_first_not_of(const wchar_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find_first_not_of(const char16_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find_first_not_of(const char32_t* s, std::size_t pos, std::size_t count) const;

        std::size_t find_first_not_of(const char* s, std::size_t pos = 0) const;
        std::size_t find_first_not_of(const wchar_t* s, std::size_t pos = 0) const;
        std::size_t find_first_not_of(const char16_t* s, std::size_t pos = 0) const;
        std::size_t find_first_not_of(const char32_t* s, std::size_t pos = 0) const;

        std::size_t find_first_not_of(char ch, std::size_t pos = 0) const noexcept;
        std::size_t find_first_not_of(wchar_t ch, std::size_t pos = 0) const noexcept;
        std::size_t find_first_not_of(char16_t ch, std::size_t pos = 0) const noexcept;
        std::size_t find_first_not_of(char32_t ch, std::size_t pos = 0) const noexcept;

        std::size_t rfind(const std::string& str, std::size_t pos = npos) const noexcept;
        std::size_t rfind(const std::wstring& str, std::size_t pos = npos) const noexcept;
        std::size_t rfind(const std::u16string& str, std::size_t pos = npos) const noexcept;
        std::size_t rfind(const std::u32string& str, std::size_t pos = npos) const noexcept;
        std::size_t rfind(const String& str, std::size_t pos = npos) const noexcept;

        std::size_t rfind(const char* s, std::size_t pos, std::size_t count) const;
        std::size_t rfind(const wchar_t* s, std::size_t pos, std::size_t count) const;
        std::size_t rfind(const char16_t* s, std::size_t pos, std::size_t count) const;
        std::size_t rfind(const char32_t* s, std::size_t pos, std::size_t count) const;

        std::size_t rfind(const char* s, std::size_t pos = npos) const;
        std::size_t rfind(const wchar_t* s, std::size_t pos = npos) const;
        std::size_t rfind(const char16_t* s, std::size_t pos = npos) const;
        std::size_t rfind(const char32_t* s, std::size_t pos = npos) const;

        std::size_t rfind(char ch, std::size_t pos = npos) const noexcept;
        std::size_t rfind(wchar_t ch, std::size_t pos = npos) const noexcept;
        std::size_t rfind(char16_t ch, std::size_t pos = npos) const noexcept;
        std::size_t rfind(char32_t ch, std::size_t pos = npos) const noexcept;

        std::size_t find_last_of(const std::string& str, std::size_t pos = npos) const noexcept;
        std::size_t find_last_of(const std::wstring& str, std::size_t pos = npos) const noexcept;
        std::size_t find_last_of(const std::u16string& str, std::size_t pos = npos) const noexcept;
        std::size_t find_last_of(const std::u32string& str, std::size_t pos = npos) const noexcept;
        std::size_t find_last_of(const String& str, std::size_t pos = npos) const noexcept;

        std::size_t find_last_of(const char* s, std::size_t pos, std::size_t count) const;
        std::size_t find_last_of(const wchar_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find_last_of(const char16_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find_last_of(const char32_t* s, std::size_t pos, std::size_t count) const;

        std::size_t find_last_of(const char* s, std::size_t pos = npos) const;
        std::size_t find_last_of(const wchar_t* s, std::size_t pos = npos) const;
        std::size_t find_last_of(const char16_t* s, std::size_t pos = npos) const;
        std::size_t find_last_of(const char32_t* s, std::size_t pos = npos) const;

        std::size_t find_last_of(char ch, std::size_t pos = npos) const noexcept;
        std::size_t find_last_of(wchar_t ch, std::size_t pos = npos) const noexcept;
        std::size_t find_last_of(char16_t ch, std::size_t pos = npos) const noexcept;
        std::size_t find_last_of(char32_t ch, std::size_t pos = npos) const noexcept;

        std::size_t find_last_not_of(const std::string& str, std::size_t pos = npos) const noexcept;
        std::size_t find_last_not_of(const std::wstring& str, std::size_t pos = npos) const noexcept;
        std::size_t find_last_not_of(const std::u16string& str, std::size_t pos = npos) const noexcept;
        std::size_t find_last_not_of(const std::u32string& str, std::size_t pos = npos) const noexcept;
        std::size_t find_last_not_of(const String& str, std::size_t pos = npos) const noexcept;

        std::size_t find_last_not_of(const char* s, std::size_t pos, std::size_t count) const;
        std::size_t find_last_not_of(const wchar_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find_last_not_of(const char16_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find_last_not_of(const char32_t* s, std::size_t pos, std::size_t count) const;

        std::size_t find_last_not_of(const char* s, std::size_t pos = npos) const;
        std::size_t find_last_not_of(const wchar_t* s, std::size_t pos = npos) const;
        std::size_t find_last_not_of(const char16_t* s, std::size_t pos = npos) const;
        std::size_t find_last_not_of(const char32_t* s, std::size_t pos = npos) const;

        std::size_t find_last_not_of(char ch, std::size_t pos = npos) const noexcept;
        std::size_t find_last_not_of(wchar_t ch, std::size_t pos = npos) const noexcept;
        std::size_t find_last_not_of(char16_t ch, std::size_t pos = npos) const noexcept;
        std::size_t find_last_not_of(char32_t ch, std::size_t pos = npos) const noexcept;

        friend bool operator==(const String& left, const String& right);
        friend bool operator!=(const String& left, const String& right);
        friend bool operator<(const String& left, const String& right);
        friend bool operator<=(const String& left, const String& right);
        friend bool operator>(const String& left, const String& right);
        friend bool operator>=(const String& left, const String& right);
        friend String operator+(const String& left, const String& right);
        friend String operator+(const String& left, String&& right);
        friend String operator+(String&& left, const String& right);
        friend String operator+(String&& left, String&& right);

#if defined(__cpp_lib_char8_t) && (__cpp_lib_char8_t >= 201811L)
        String(const std::u8string& str);
        String(char8_t utfChar);
        String(const char8_t* str);
        String(std::size_t count, char8_t ch);
        String(const std::u8string& str, std::size_t pos);
        String(const std::u8string& str, std::size_t pos, std::size_t count);
        String(const char8_t* str, std::size_t count);
        String(std::initializer_list<char8_t> chars);
        String(std::u8string::const_iterator first, std::u8string::const_iterator last);

        explicit operator std::u8string() const;

        std::u8string toUtf8() const;

        String& assign(std::size_t count, char8_t ch);
        String& assign(const std::u8string& str);
        String& assign(const std::u8string& str, std::size_t pos, std::size_t count = npos);
        String& assign(std::u8string&& str);
        String& assign(const char8_t* str, std::size_t count);
        String& assign(const char8_t* str);
        String& assign(std::initializer_list<char8_t> chars);
        String& assign(std::u8string::const_iterator first, std::u8string::const_iterator last);

        String& insert(std::size_t index, std::size_t count, char8_t ch);
        String& insert(std::size_t index, const std::u8string& str);
        String& insert(std::size_t index, const std::u8string& str, std::size_t pos, std::size_t count = npos);
        String& insert(std::size_t index, const char8_t* str, std::size_t count);
        String& insert(std::size_t index, const char8_t* str);
        iterator insert(const_iterator pos, char8_t ch);
        iterator insert(const_iterator pos, std::size_t count, char8_t ch);
        iterator insert(const_iterator pos, std::initializer_list<char8_t> chars);
        iterator insert(const_iterator pos, std::u8string::const_iterator first, std::u8string::const_iterator last);

        String& append(std::size_t count, char8_t ch);
        String& append(const std::u8string& str);
        String& append(const std::u8string& str, std::size_t pos, std::size_t count = npos);
        String& append(const char8_t* str, std::size_t count);
        String& append(const char8_t* str);
        String& append(std::initializer_list<char8_t> chars);
        String& append(std::u8string::const_iterator first, std::u8string::const_iterator last);

        int compare(const std::u8string& str) const noexcept;
        int compare(std::size_t pos1, std::size_t count1, const std::u8string& str) const;
        int compare(std::size_t pos1, std::size_t count1, const std::u8string& str, std::size_t pos2, std::size_t count2 = npos) const;
        int compare(const char8_t* s) const;
        int compare(std::size_t pos1, std::size_t count1, const char8_t* s) const;
        int compare(std::size_t pos1, std::size_t count1, const char8_t* s, std::size_t count2) const;

        String& replace(std::size_t pos, std::size_t count, const std::u8string& str);
        String& replace(const_iterator first, const_iterator last, const std::u8string& str);
        String& replace(std::size_t pos, std::size_t count, const std::u8string& str, std::size_t pos2, std::size_t count2 = npos);
        String& replace(const_iterator first, const_iterator last, std::u8string::const_iterator first2, std::u8string::const_iterator last2);
        String& replace(std::size_t pos, std::size_t count, const char8_t* cstr, std::size_t count2);
        String& replace(const_iterator first, const_iterator last, const char8_t* cstr, std::size_t count2);
        String& replace(std::size_t pos, std::size_t count, const char8_t* cstr);
        String& replace(const_iterator first, const_iterator last, const char8_t* cstr);
        String& replace(std::size_t pos, std::size_t count, std::size_t count2, char8_t ch);
        String& replace(const_iterator first, const_iterator last, std::size_t count2, char8_t ch);
        String& replace(const_iterator first, const_iterator last, std::initializer_list<char8_t> chars);

        void resize(std::size_t count, char8_t ch);

        bool contains(char8_t c) const noexcept;
        bool contains(const char8_t* s) const noexcept;

        std::size_t find(const std::u8string& str, std::size_t pos = 0) const noexcept;
        std::size_t find(const char8_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find(const char8_t* s, std::size_t pos = 0) const;
        std::size_t find(char8_t ch, std::size_t pos = 0) const noexcept;

        std::size_t find_first_of(const std::u8string& str, std::size_t pos = 0) const noexcept;
        std::size_t find_first_of(const char8_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find_first_of(const char8_t* s, std::size_t pos = 0) const;
        std::size_t find_first_of(char8_t ch, std::size_t pos = 0) const noexcept;

        std::size_t find_first_not_of(const std::u8string& str, std::size_t pos = 0) const noexcept;
        std::size_t find_first_not_of(const char8_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find_first_not_of(const char8_t* s, std::size_t pos = 0) const;
        std::size_t find_first_not_of(char8_t ch, std::size_t pos = 0) const noexcept;

        std::size_t rfind(const std::u8string& str, std::size_t pos = npos) const noexcept;
        std::size_t rfind(const char8_t* s, std::size_t pos, std::size_t count) const;
        std::size_t rfind(const char8_t* s, std::size_t pos = npos) const;
        std::size_t rfind(char8_t ch, std::size_t pos = npos) const noexcept;

        std::size_t find_last_of(const std::u8string& str, std::size_t pos = npos) const noexcept;
        std::size_t find_last_of(const char8_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find_last_of(const char8_t* s, std::size_t pos = npos) const;
        std::size_t find_last_of(char8_t ch, std::size_t pos = npos) const noexcept;

        std::size_t find_last_not_of(const std::u8string& str, std::size_t pos = npos) const noexcept;
        std::size_t find_last_not_of(const char8_t* s, std::size_t pos, std::size_t count) const;
        std::size_t find_last_not_of(const char8_t* s, std::size_t pos = npos) const;
        std::size_t find_last_not_of(char8_t ch, std::size_t pos = npos) const noexcept;

        //friend std::basic_ostream<char8_t>& operator<<(std::basic_ostream<char8_t>& os, const String& str);
#endif
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    inline bool operator==(const String& left, const String& right)
    {
        return left.m_string == right.m_string;
    }

    inline bool operator!=(const String& left, const String& right)
    {
        return left.m_string != right.m_string;
    }

    inline bool operator<(const String& left, const String& right)
    {
        return left.m_string < right.m_string;
    }

    inline bool operator<=(const String& left, const String& right)
    {
        return left.m_string <= right.m_string;
    }

    inline bool operator>(const String& left, const String& right)
    {
        return left.m_string > right.m_string;
    }

    inline bool operator>=(const String& left, const String& right)
    {
        return left.m_string >= right.m_string;
    }

    inline String operator+(const String& left, const String& right)
    {
        return String(left.m_string + right.m_string);
    }
    inline String operator+(String&& left, String&& right)
    {
        return String(std::move(left.m_string) + std::move(right.m_string));
    }
    inline String operator+(String&& left, const String& right)
    {
        return String(std::move(left.m_string) + right.m_string);
    }
    inline String operator+(const String& left, String&& right)
    {
        return String(left.m_string + std::move(right.m_string));
    }

    std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const String& str);
    std::basic_ostream<wchar_t>& operator<<(std::basic_ostream<wchar_t>& os, const String& str);
    //SP_API std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, const String& str);
    //SP_API std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const String& str);


    // UTF-8 function are defined in the header so that they can be enabled/disabled based on
    // the compiler settings without having to recompile SP with a different c++ standard.
#if defined(__cpp_lib_char8_t) && (__cpp_lib_char8_t >= 201811L)
    inline String::String(const std::u8string& str) :
        m_string(utf::convertUtf8toUtf32(str.begin(), str.end()))
    {
    }

    inline String::String(char8_t utfChar)
        : m_string(1, static_cast<char32_t>(utfChar))
    {
    }

    inline String::String(const char8_t* str)
        : String{utf::convertUtf8toUtf32(str, str + std::char_traits<char8_t>::length(str))}
    {
    }

    inline String::String(std::size_t count, char8_t ch)
        : m_string(count, static_cast<char32_t>(ch))
    {
    }

    inline String::String(const std::u8string& str, std::size_t pos)
        : String{std::u8string(str, pos)}
    {
    }

    inline String::String(const std::u8string& str, std::size_t pos, std::size_t count)
        : String{std::u8string(str, pos, count)}
    {
    }

    inline String::String(const char8_t* str, std::size_t count)
        : String{std::u8string{str, count}}
    {
    }

    inline String::String(std::initializer_list<char8_t> chars)
        : String(std::u8string(chars.begin(), chars.end()))
    {
    }

    inline String::String(std::u8string::const_iterator first, std::u8string::const_iterator last)
        : String{std::u8string(first, last)}
    {
    }

    inline String::operator std::u8string() const
    {
        return toUtf8();
    }

    inline std::u8string String::toUtf8() const
    {
        return utf::convertUtf32toUtf8(m_string);
    }

    inline String& String::assign(std::size_t count, char8_t ch)
    {
        m_string.assign(count, static_cast<char32_t>(ch));
        return *this;
    }

    inline String& String::assign(const std::u8string& str)
    {
        return *this = str;
    }

    inline String& String::assign(const std::u8string& str, std::size_t pos, std::size_t count)
    {
        return *this = {str, pos, count};
    }

    inline String& String::assign(std::u8string&& str)
    {
        return *this = std::move(str);
    }

    inline String& String::assign(const char8_t* str, std::size_t count)
    {
        return *this = {str, count};
    }

    inline String& String::assign(const char8_t* str)
    {
        return *this = str;
    }

    inline String& String::assign(std::initializer_list<char8_t> chars)
    {
        return *this = chars;
    }

    inline String& String::assign(std::u8string::const_iterator first, std::u8string::const_iterator last)
    {
        return *this = {first, last};
    }

    inline String& String::insert(std::size_t index, std::size_t count, char8_t ch)
    {
        m_string.insert(index, count, static_cast<char32_t>(ch));
        return *this;
    }

    inline String& String::insert(std::size_t index, const std::u8string& str)
    {
        m_string.insert(index, String{str}.m_string);
        return *this;
    }

    inline String& String::insert(std::size_t index, const std::u8string& str, std::size_t pos, std::size_t count)
    {
        m_string.insert(index, String{str, pos, count}.m_string);
        return *this;
    }

    inline String& String::insert(std::size_t index, const char8_t* str, std::size_t count)
    {
        m_string.insert(index, String{str, count}.m_string);
        return *this;
    }

    inline String& String::insert(std::size_t index, const char8_t* str)
    {
        m_string.insert(index, String{str}.m_string);
        return *this;
    }

    inline String::iterator String::insert(String::const_iterator pos, char8_t ch)
    {
        return m_string.insert(pos, static_cast<char32_t>(ch));
    }

    inline String::iterator String::insert(String::const_iterator pos, std::size_t count, char8_t ch)
    {
        return m_string.insert(pos, count, static_cast<char32_t>(ch));
    }

    inline String::iterator String::insert(String::const_iterator pos, std::initializer_list<char8_t> chars)
    {
        auto tmpStr = String{chars};
        return m_string.insert(pos, tmpStr.begin(), tmpStr.end());
    }

    inline String::iterator String::insert(String::const_iterator pos, std::u8string::const_iterator first, std::u8string::const_iterator last)
    {
        auto tmpStr = String{first, last};
        return m_string.insert(pos, tmpStr.begin(), tmpStr.end());
    }

    inline String& String::append(std::size_t count, char8_t ch)
    {
        return append(String(count, ch));
    }

    inline String& String::append(const std::u8string& str)
    {
        return append(String{str});
    }

    inline String& String::append(const std::u8string& str, std::size_t pos, std::size_t count)
    {
        return append(String{str, pos, count});
    }

    inline String& String::append(const char8_t* str, std::size_t count)
    {
        return append(String{str, count});
    }

    inline String& String::append(const char8_t* str)
    {
        return append(String{str});
    }

    inline String& String::append(std::initializer_list<char8_t> chars)
    {
        return append(String{chars});
    }

    inline String& String::append(std::u8string::const_iterator first, std::u8string::const_iterator last)
    {
        return append(String{first, last});
    }

    inline int String::compare(const std::u8string& str) const noexcept
    {
        return m_string.compare(String{str}.m_string);
    }

    inline int String::compare(std::size_t pos1, std::size_t count1, const std::u8string& str) const
    {
        return m_string.compare(pos1, count1, String{str}.m_string);
    }

    inline int String::compare(std::size_t pos1, std::size_t count1, const std::u8string& str, std::size_t pos2, std::size_t count2) const
    {
        return m_string.compare(pos1, count1, String{str, pos2, count2}.m_string);
    }

    inline int String::compare(const char8_t* s) const
    {
        return m_string.compare(String{s}.m_string);
    }

    inline int String::compare(std::size_t pos1, std::size_t count1, const char8_t* s) const
    {
        return m_string.compare(pos1, count1, String{s}.m_string);
    }

    inline int String::compare(std::size_t pos1, std::size_t count1, const char8_t* s, std::size_t count2) const
    {
        return m_string.compare(pos1, count1, String{s, count2}.m_string);
    }

    inline String& String::replace(std::size_t pos, std::size_t count, const std::u8string& str)
    {
        m_string.replace(pos, count, String{str}.m_string);
        return *this;
    }

    inline String& String::replace(const_iterator first, const_iterator last, const std::u8string& str)
    {
        m_string.replace(first, last, String{str}.m_string);
        return *this;
    }

    inline String& String::replace(std::size_t pos, std::size_t count, const std::u8string& str, std::size_t pos2, std::size_t count2)
    {
        m_string.replace(pos, count, String{str, pos2, count2}.m_string);
        return *this;
    }

    inline String& String::replace(const_iterator first, const_iterator last, std::u8string::const_iterator first2, std::u8string::const_iterator last2)
    {
        m_string.replace(first, last, String{first2, last2}.m_string);
        return *this;
    }

    inline String& String::replace(std::size_t pos, std::size_t count, const char8_t* cstr, std::size_t count2)
    {
        m_string.replace(pos, count, String{cstr, count2}.m_string);
        return *this;
    }

    inline String& String::replace(const_iterator first, const_iterator last, const char8_t* cstr, std::size_t count2)
    {
        m_string.replace(first, last, String{cstr, count2}.m_string);
        return *this;
    }

    inline String& String::replace(std::size_t pos, std::size_t count, const char8_t* cstr)
    {
        m_string.replace(pos, count, String{cstr}.m_string);
        return *this;
    }

    inline String& String::replace(const_iterator first, const_iterator last, const char8_t* cstr)
    {
        m_string.replace(first, last, String{cstr}.m_string);
        return *this;
    }

    inline String& String::replace(std::size_t pos, std::size_t count, std::size_t count2, char8_t ch)
    {
        m_string.replace(pos, count, String(count2, ch).m_string);
        return *this;
    }

    inline String& String::replace(const_iterator first, const_iterator last, std::size_t count2, char8_t ch)
    {
        m_string.replace(first, last, String(count2, ch).m_string);
        return *this;
    }

    inline String& String::replace(const_iterator first, const_iterator last, std::initializer_list<char8_t> chars)
    {
        m_string.replace(first, last, String{chars}.m_string);
        return *this;
    }

    inline void String::resize(std::size_t count, char8_t ch)
    {
        m_string.resize(count, static_cast<char32_t>(ch));
    }

    inline bool String::contains(char8_t c) const noexcept
    {
        return contains(static_cast<char32_t>(c));
    }

    inline bool String::contains(const char8_t* s) const noexcept
    {
        return find(s) != npos;
    }

    inline std::size_t String::find(const std::u8string& str, std::size_t pos) const noexcept
    {
        return m_string.find(String{str}.m_string, pos);
    }

    inline std::size_t String::find(const char8_t* s, std::size_t pos, std::size_t count) const
    {
        return m_string.find(String{s, count}.m_string, pos);
    }

    inline std::size_t String::find(const char8_t* s, std::size_t pos) const
    {
        return m_string.find(String{s}.m_string, pos);
    }

    inline std::size_t String::find(char8_t ch, std::size_t pos) const noexcept
    {
        return m_string.find(static_cast<char32_t>(ch), pos);
    }

    inline std::size_t String::find_first_of(const std::u8string& str, std::size_t pos) const noexcept
    {
        return m_string.find_first_of(String{str}.m_string, pos);
    }

    inline std::size_t String::find_first_of(const char8_t* s, std::size_t pos, std::size_t count) const
    {
        return m_string.find_first_of(String{s, count}.m_string, pos);
    }

    inline std::size_t String::find_first_of(const char8_t* s, std::size_t pos) const
    {
        return m_string.find_first_of(String{s}.m_string, pos);
    }

    inline std::size_t String::find_first_of(char8_t ch, std::size_t pos) const noexcept
    {
        return m_string.find_first_of(static_cast<char32_t>(ch), pos);
    }

    inline std::size_t String::find_first_not_of(const std::u8string& str, std::size_t pos) const noexcept
    {
        return m_string.find_first_not_of(String{str}.m_string, pos);
    }

    inline std::size_t String::find_first_not_of(const char8_t* s, std::size_t pos, std::size_t count) const
    {
        return m_string.find_first_not_of(String{s, count}.m_string, pos);
    }

    inline std::size_t String::find_first_not_of(const char8_t* s, std::size_t pos) const
    {
        return m_string.find_first_not_of(String{s}.m_string, pos);
    }

    inline std::size_t String::find_first_not_of(char8_t ch, std::size_t pos) const noexcept
    {
        return m_string.find_first_not_of(static_cast<char32_t>(ch), pos);
    }

    inline std::size_t String::rfind(const std::u8string& str, std::size_t pos) const noexcept
    {
        return m_string.rfind(String{str}.m_string, pos);
    }

    inline std::size_t String::rfind(const char8_t* s, std::size_t pos, std::size_t count) const
    {
        return m_string.rfind(String{s, count}.m_string, pos);
    }

    inline std::size_t String::rfind(const char8_t* s, std::size_t pos) const
    {
        return m_string.rfind(String{s}.m_string, pos);
    }

    inline std::size_t String::rfind(char8_t ch, std::size_t pos) const noexcept
    {
        return m_string.rfind(static_cast<char32_t>(ch), pos);
    }

    inline std::size_t String::find_last_of(const std::u8string& str, std::size_t pos) const noexcept
    {
        return m_string.find_last_of(String{str}.m_string, pos);
    }

    inline std::size_t String::find_last_of(const char8_t* s, std::size_t pos, std::size_t count) const
    {
        return m_string.find_last_of(String{s, count}.m_string, pos);
    }

    inline std::size_t String::find_last_of(const char8_t* s, std::size_t pos) const
    {
        return m_string.find_last_of(String{s}.m_string, pos);
    }

    inline std::size_t String::find_last_of(char8_t ch, std::size_t pos) const noexcept
    {
        return m_string.find_last_of(static_cast<char32_t>(ch), pos);
    }

    inline std::size_t String::find_last_not_of(const std::u8string& str, std::size_t pos) const noexcept
    {
        return m_string.find_last_not_of(String{str}.m_string, pos);
    }

    inline std::size_t String::find_last_not_of(const char8_t* s, std::size_t pos, std::size_t count) const
    {
        return m_string.find_last_not_of(String{s, count}.m_string, pos);
    }

    inline std::size_t String::find_last_not_of(const char8_t* s, std::size_t pos) const
    {
        return m_string.find_last_not_of(String{s}.m_string, pos);
    }

    inline std::size_t String::find_last_not_of(char8_t ch, std::size_t pos) const noexcept
    {
        return m_string.find_last_not_of(static_cast<char8_t>(ch), pos);
    }

    // Doesn't compile with libc++
    //inline std::basic_ostream<char8_t>& operator<<(std::basic_ostream<char8_t>& os, const String& str)
    //{
    //    os << std::u8string(str);
    //    return os;
    //}
#endif

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if SP_COMPILE_VERSION >= 17
    using StringView = std::u32string_view;
    using CharStringView = std::string_view;
#else
    using StringView = String;
    using CharStringView = std::string;
#endif

}
#endif
