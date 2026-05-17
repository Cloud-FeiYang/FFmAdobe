'''
''' PremierePresetConverter
''' 
''' Reads a full FFmpegFreeUI preset JSON (180+ fields with Chinese property names)
''' and extracts only the encoding-relevant fields into a simplified JSON format
''' that the Premiere Pro C++ plugin can parse.
'''
''' Usage: PremierePresetConverter.exe <input_full_preset.json> <output_simplified.json>
'''
''' This tool lives OUTSIDE the FFmpegFreeUI git subtree to preserve upstream sync.
'''
Imports System.IO
Imports System.Text.Json

Module Program

    Sub Main(args As String())
        If args.Length < 2 Then
            Console.Error.WriteLine("Usage: PremierePresetConverter <input.json> <output.json>")
            Environment.ExitCode = 1
            Exit Sub
        End If

        Dim inputPath = args(0)
        Dim outputPath = args(1)

        If Not File.Exists(inputPath) Then
            Console.Error.WriteLine($"Input file not found: {inputPath}")
            Environment.ExitCode = 2
            Exit Sub
        End If

        Try
            Dim jsonText = File.ReadAllText(inputPath)
            Dim doc = JsonDocument.Parse(jsonText)
            Dim root = doc.RootElement

            ' Build simplified JSON
            Dim videoArgs = BuildVideoArgs(root)
            Dim audioArgs = BuildAudioArgs(root)
            Dim videoFilters = BuildVideoFilters(root)
            Dim audioFilters = BuildAudioFilters(root)
            Dim container = GetStr(root, "输出容器")
            Dim extraInputArgs = BuildExtraInputArgs(root)

            ' Write simplified JSON
            Dim opts As New JsonSerializerOptions With {
                .WriteIndented = True,
                .Encoder = Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping
            }
            Dim result As New Dictionary(Of String, String) From {
                {"version", "1"},
                {"video_args", videoArgs.Trim()},
                {"audio_args", audioArgs.Trim()},
                {"video_filters", videoFilters.Trim()},
                {"audio_filters", audioFilters.Trim()},
                {"container", container},
                {"extra_input_args", extraInputArgs.Trim()}
            }

            Directory.CreateDirectory(Path.GetDirectoryName(outputPath))
            File.WriteAllText(outputPath, JsonSerializer.Serialize(result, opts))
            Console.WriteLine($"OK: {outputPath}")

        Catch ex As Exception
            Console.Error.WriteLine($"Error: {ex.Message}")
            Environment.ExitCode = 3
        End Try
    End Sub

    ' ========== Helper: safely read JSON string property ==========
    Function GetStr(root As JsonElement, name As String) As String
        Dim el As JsonElement
        If root.TryGetProperty(name, el) AndAlso el.ValueKind = JsonValueKind.String Then
            Return el.GetString()
        End If
        Return ""
    End Function

    Function GetInt(root As JsonElement, name As String) As Integer
        Dim el As JsonElement
        If root.TryGetProperty(name, el) Then
            If el.ValueKind = JsonValueKind.Number Then Return el.GetInt32()
            If el.ValueKind = JsonValueKind.String Then
                Dim v As Integer
                If Integer.TryParse(el.GetString(), v) Then Return v
            End If
        End If
        Return 0
    End Function

    Function GetBool(root As JsonElement, name As String) As Boolean
        Dim el As JsonElement
        If root.TryGetProperty(name, el) Then
            If el.ValueKind = JsonValueKind.True Then Return True
            If el.ValueKind = JsonValueKind.False Then Return False
        End If
        Return False
    End Function

    Function GetStringArray(root As JsonElement, name As String) As List(Of String)
        Dim result As New List(Of String)
        Dim el As JsonElement
        If root.TryGetProperty(name, el) AndAlso el.ValueKind = JsonValueKind.Array Then
            For Each item In el.EnumerateArray()
                If item.ValueKind = JsonValueKind.String Then result.Add(item.GetString())
            Next
        End If
        Return result
    End Function

    ' ========== Video encoding arguments ==========
    Function BuildVideoArgs(r As JsonElement) As String
        Dim s = ""
        ' Check for "完全自己写" mode
        Dim custom = GetStr(r, "自定义参数_完全自己写")
        If custom <> "" Then
            Return custom ' User wrote entire command manually
        End If

        Dim codec = GetStr(r, "视频参数_编码器_具体编码")
        Dim category = GetStr(r, "视频参数_编码器_类别")
        If category = "禁用" Then Return "-vn"
        If codec <> "" Then s &= $"-c:v {codec} "

        ' Preset
        Dim preset = GetStr(r, "视频参数_编码器_编码预设")
        If preset <> "" Then
            Select Case codec
                Case "libaom-av1", "libvpx-vp9"
                    s &= $"-cpu-used {preset} "
                Case Else
                    s &= $"-preset {preset} "
            End Select
        End If

        ' Profile
        Dim profile = GetStr(r, "视频参数_编码器_配置文件")
        If profile <> "" Then s &= $"-profile:v {profile} "

        ' Tune / usage / deadline
        Dim tune = GetStr(r, "视频参数_编码器_场景优化")
        If tune <> "" Then
            Select Case codec
                Case "hevc_amf", "h264_amf" : s &= $"-usage {tune} "
                Case "libvpx-vp9" : s &= $"-deadline {tune} "
                Case Else : s &= $"-tune {tune} "
            End Select
        End If

        ' GPU
        Dim gpu = GetStr(r, "视频参数_编码器_gpu")
        If gpu <> "" Then s &= $"-gpu {gpu} "

        ' Threads
        Dim threads = GetStr(r, "视频参数_编码器_threads")
        If threads <> "" Then s &= $"-threads {threads} "

        ' Rate control
        Dim rcMode = GetStr(r, "视频参数_比特率_控制方式")
        Select Case rcMode
            Case "VBR"
                Select Case codec
                    Case "av1_amf", "hevc_amf", "h264_amf" : s &= "-rc qvbr "
                    Case Else : s &= "-rc vbr "
                End Select
            Case "VBR HQ"
                Select Case codec
                    Case "hevc_nvenc", "h264_nvenc" : s &= "-rc vbr_hq "
                    Case "av1_amf" : s &= "-rc hqvbr -quality high_quality "
                    Case "hevc_amf", "h264_amf" : s &= "-rc hqvbr -quality quality "
                    Case "av1_qsv", "hevc_qsv", "h264_qsv" : s &= "-rc la_icq "
                End Select
            Case "CQP"
                Select Case codec
                    Case "av1_nvenc", "hevc_nvenc", "h264_nvenc" : s &= "-rc constqp "
                    Case "av1_amf", "hevc_amf", "h264_amf" : s &= "-rc cqp "
                End Select
            Case "CBR" : s &= "-rc cbr "
        End Select

        ' Quality value
        Dim qName = GetStr(r, "视频参数_质量控制_参数名")
        Dim qVal = GetStr(r, "视频参数_质量控制_值")
        If qVal <> "" Then
            Select Case qName
                Case "crf" : s &= $"-crf {qVal} "
                Case "cq" : s &= $"-cq {qVal} "
                Case "qp" : s &= $"-qp {qVal} "
                Case "global_quality" : s &= $"-global_quality {qVal} "
            End Select
        End If

        ' Bitrate
        Dim br = GetStr(r, "视频参数_比特率_基础")
        If br <> "" Then s &= $"-b:v {br} "
        Dim brMin = GetStr(r, "视频参数_比特率_最低值")
        If brMin <> "" Then s &= $"-minrate {brMin} "
        Dim brMax = GetStr(r, "视频参数_比特率_最高值")
        If brMax <> "" Then s &= $"-maxrate {brMax} "
        Dim bufsize = GetStr(r, "视频参数_比特率_缓冲区")
        If bufsize <> "" Then s &= $"-bufsize {bufsize} "

        ' Advanced quality params
        For Each p In GetStringArray(r, "视频参数_质量控制_进阶参数集")
            s &= $"{p} "
        Next

        ' Pixel format
        Dim pixfmt = GetStr(r, "视频参数_色彩管理_像素格式")
        If pixfmt <> "" Then s &= $"-pix_fmt {pixfmt} "

        ' Color management (output side only: metadata flags)
        Dim colorMode = GetInt(r, "视频参数_色彩管理_处理方式")
        If colorMode = 1 OrElse colorMode = 2 Then
            Dim cs = GetStr(r, "视频参数_色彩管理_矩阵系数")
            If cs <> "" Then s &= $"-colorspace {cs} "
            Dim cp = GetStr(r, "视频参数_色彩管理_色域")
            If cp <> "" Then s &= $"-color_primaries {cp} "
            Dim ct = GetStr(r, "视频参数_色彩管理_传输特性")
            If ct <> "" Then s &= $"-color_trc {ct} "
            Dim cr = GetStr(r, "视频参数_色彩管理_范围")
            If cr <> "" Then s &= $"-color_range {cr} "
        End If

        ' Custom video params
        Dim customV = GetStr(r, "自定义参数_视频参数")
        If customV <> "" Then s &= $"{customV} "

        Return s
    End Function

    ' ========== Audio encoding arguments ==========
    Function BuildAudioArgs(r As JsonElement) As String
        Dim s = ""
        Dim codec = GetStr(r, "音频参数_编码器_具体编码")
        If codec <> "" Then
            If codec = "-an" Then Return "-an"
            s &= $"-c:a {codec} "
        End If
        Dim br = GetStr(r, "音频参数_比特率")
        If br <> "" Then s &= $"-b:a {br} "
        Dim qName = GetStr(r, "音频参数_质量参数名")
        Dim qVal = GetStr(r, "音频参数_质量值")
        If qName <> "" AndAlso qVal <> "" Then s &= $"{qName} {qVal} "
        Dim ch = GetStr(r, "音频参数_声道数")
        If ch <> "" Then s &= $"-channel_layout {ch} "
        Dim sr = GetStr(r, "音频参数_采样率")
        If sr <> "" Then s &= $"-ar {sr} "

        Dim customA = GetStr(r, "自定义参数_音频参数")
        If customA <> "" Then s &= $"{customA} "
        Return s
    End Function

    ' ========== Video filters ==========
    Function BuildVideoFilters(r As JsonElement) As String
        Dim filters As New List(Of String)

        ' Deinterlace / field order
        Dim interlace = GetInt(r, "视频参数_逐行与隔行")
        Select Case interlace
            Case 1 : filters.Add("yadif=0:-1:0")
            Case 2 : filters.Add("yadif=0:0:0")
            Case 3 : filters.Add("yadif=0:1:0")
            Case 4 : filters.Add("tinterlace=4")
            Case 5 : filters.Add("tinterlace=6")
            Case 6 : filters.Add("fieldmatch,yadif=deint=interlaced,decimate")
            Case 7 : filters.Add("yadif=1")
            Case 8 : filters.Add("pullup=jl=1:jr=1,fps=25")
            Case 9 : filters.Add("yadif=0")
            Case 10 : filters.Add("yadif=1")
            Case 11 : filters.Add("bwdif=0")
            Case 12 : filters.Add("bwdif=1")
        End Select

        ' Resolution (scale filter)
        Dim autoW = GetStr(r, "视频参数_分辨率自动计算_宽度")
        Dim autoH = GetStr(r, "视频参数_分辨率自动计算_高度")
        If autoW <> "" Then
            filters.Add($"scale={autoW}:-2")
        ElseIf autoH <> "" Then
            filters.Add($"scale=-2:{autoH}")
        End If

        ' Denoise
        Dim denoise = GetStr(r, "视频参数_降噪_方式")
        Dim d1 = GetStr(r, "视频参数_降噪_参数1")
        Dim d2 = GetStr(r, "视频参数_降噪_参数2")
        Dim d3 = GetStr(r, "视频参数_降噪_参数3")
        Dim d4 = GetStr(r, "视频参数_降噪_参数4")
        Select Case denoise
            Case "hqdn3d"
                Dim p As New List(Of String)
                If d1 <> "" Then p.Add($"luma_spatial={d1}")
                If d2 <> "" Then p.Add($"chroma_spatial={d2}")
                If d3 <> "" Then p.Add($"luma_tmp={d3}")
                If d4 <> "" Then p.Add($"chroma_tmp={d4}")
                If p.Count > 0 Then filters.Add($"hqdn3d={String.Join(":", p)}")
            Case "nlmeans"
                Dim p As New List(Of String)
                If d1 <> "" Then p.Add($"s={d1}")
                If d2 <> "" Then p.Add($"p={d2}")
                If d3 <> "" Then p.Add($"pc={d3}")
                If d4 <> "" Then p.Add($"r={d4}")
                If p.Count > 0 Then filters.Add($"nlmeans={String.Join(":", p)}")
            Case "atadenoise"
                Dim p As New List(Of String)
                If d1 <> "" Then p.Add($"0a={d1}")
                If d2 <> "" Then p.Add($"0b={d2}")
                If d3 <> "" Then p.Add($"1a={d3}")
                If d4 <> "" Then p.Add($"1b={d4}")
                If p.Count > 0 Then filters.Add($"atadenoise={String.Join(":", p)}")
            Case "bm3d"
                Dim p As New List(Of String)
                If d1 <> "" Then p.Add($"sigma={d1}")
                If d2 <> "" Then p.Add($"block={d2}")
                If d3 <> "" Then p.Add($"bstep={d3}")
                If d4 <> "" Then p.Add($"group={d4}")
                If p.Count > 0 Then filters.Add($"bm3d={String.Join(":", p)}")
        End Select

        ' Sharpen
        Dim shX = GetStr(r, "视频参数_锐化_水平尺寸")
        Dim shY = GetStr(r, "视频参数_锐化_垂直尺寸")
        Dim shA = GetStr(r, "视频参数_锐化_锐化强度")
        If shX <> "" AndAlso shY <> "" AndAlso shA <> "" Then
            filters.Add($"unsharp=luma_msize_x={shX}:luma_msize_y={shY}:luma_amount={shA}")
        End If

        ' Rotation
        Dim rotate = GetInt(r, "视频参数_画面翻转_角度翻转")
        Select Case rotate
            Case 1 : filters.Add("transpose=1")
            Case 2 : filters.AddRange({"transpose=1", "transpose=1"})
            Case 3 : filters.AddRange({"transpose=1", "transpose=1", "transpose=1"})
            Case 4 : filters.Add("transpose=2")
            Case 5 : filters.AddRange({"transpose=2", "transpose=2"})
            Case 6 : filters.AddRange({"transpose=2", "transpose=2", "transpose=2"})
        End Select

        ' Mirror
        Dim mirror = GetInt(r, "视频参数_画面翻转_镜像翻转")
        Select Case mirror
            Case 1 : filters.Add("hflip")
            Case 2 : filters.Add("vflip")
        End Select

        ' Color management filters
        Dim colorMode = GetInt(r, "视频参数_色彩管理_处理方式")
        If colorMode = 1 OrElse colorMode = 3 Then
            Dim filterType = GetStr(r, "视频参数_色彩管理_滤镜选择")
            Select Case filterType
                Case "zscale"
                    Dim p As New List(Of String)
                    Dim m = GetStr(r, "视频参数_色彩管理_矩阵系数") : If m <> "" Then p.Add($"matrix={m}")
                    Dim pr = GetStr(r, "视频参数_色彩管理_色域") : If pr <> "" Then p.Add($"primaries={pr}")
                    Dim tr = GetStr(r, "视频参数_色彩管理_传输特性") : If tr <> "" Then p.Add($"transfer={tr}")
                    Dim rg = GetStr(r, "视频参数_色彩管理_范围")
                    If rg = "pc" Then p.Add("range=full")
                    If rg = "tv" Then p.Add("range=limited")
                    If p.Count > 0 Then filters.Add($"zscale={String.Join(":", p)}")
                Case "libplacebo"
                    Dim p As New List(Of String)
                    Dim m = GetStr(r, "视频参数_色彩管理_矩阵系数") : If m <> "" Then p.Add($"colorspace={m}")
                    Dim pr = GetStr(r, "视频参数_色彩管理_色域") : If pr <> "" Then p.Add($"color_primaries={pr}")
                    Dim tr = GetStr(r, "视频参数_色彩管理_传输特性") : If tr <> "" Then p.Add($"color_trc={tr}")
                    Dim rg = GetStr(r, "视频参数_色彩管理_范围")
                    If rg = "pc" Then p.Add("range=full")
                    If rg = "tv" Then p.Add("range=limited")
                    Dim tm = GetStr(r, "视频参数_色彩管理_色调映射算法")
                    If tm <> "" Then p.Add($"tonemapping={tm}")
                    If p.Count > 0 Then filters.Add($"libplacebo={String.Join(":", p)}")
            End Select
        End If

        ' EQ (brightness/contrast/saturation/gamma)
        Dim br2 = GetStr(r, "视频参数_色彩管理_亮度")
        Dim ct2 = GetStr(r, "视频参数_色彩管理_对比度")
        Dim st2 = GetStr(r, "视频参数_色彩管理_饱和度")
        Dim gm2 = GetStr(r, "视频参数_色彩管理_伽马")
        If br2 <> "" AndAlso ct2 <> "" AndAlso st2 <> "" AndAlso gm2 <> "" Then
            Dim eq As New List(Of String)
            eq.Add($"brightness={br2}") : eq.Add($"contrast={ct2}")
            eq.Add($"saturation={st2}") : eq.Add($"gamma={gm2}")
            filters.Add($"eq={String.Join(":", eq)}")
        End If

        ' Custom video filter
        Dim customVF = GetStr(r, "自定义参数_视频滤镜")
        If customVF <> "" Then filters.Add(customVF)

        Return String.Join(",", filters)
    End Function

    ' ========== Audio filters ==========
    Function BuildAudioFilters(r As JsonElement) As String
        Dim filters As New List(Of String)

        Dim loudTarget = GetStr(r, "音频参数_响度标准化_目标响度")
        If loudTarget <> "" Then
            Dim lra = GetStr(r, "音频参数_响度标准化_动态范围")
            Dim tp = GetStr(r, "音频参数_响度标准化_峰值电平")
            filters.Add($"loudnorm=I={If(loudTarget <> "", loudTarget, "-16")}:LRA={If(lra <> "", lra, "1")}:tp={If(tp <> "", tp, "-1")}")
        End If

        Dim customAF = GetStr(r, "自定义参数_音频滤镜")
        If customAF <> "" Then filters.Add(customAF)

        Return String.Join(",", filters)
    End Function

    ' ========== Extra input arguments (decoding) ==========
    Function BuildExtraInputArgs(r As JsonElement) As String
        Dim s = ""
        Dim decoder = GetStr(r, "解码参数_解码器")
        If decoder <> "" Then s &= $"-hwaccel {decoder} "
        Dim decThreads = GetStr(r, "解码参数_CPU解码线程数")
        If decThreads <> "" Then s &= $"-threads {decThreads} "
        Dim decFmt = GetStr(r, "解码参数_解码数据格式")
        If decFmt <> "" Then s &= $"-hwaccel_output_format {decFmt} "
        Dim hwName = GetStr(r, "解码参数_指定硬件的参数名")
        Dim hwVal = GetStr(r, "解码参数_指定硬件的参数")
        If hwName <> "" AndAlso hwVal <> "" Then s &= $"{hwName} {hwVal} "

        Dim customBefore = GetStr(r, "自定义参数_开头参数")
        If customBefore <> "" Then s &= $"{customBefore} "
        Return s
    End Function

End Module
